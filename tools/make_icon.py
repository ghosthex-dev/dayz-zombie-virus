"""Make ZombieVirus/data/zv_biohazard_ca.paa: white biohazard symbol with a "Z" in the middle, transparent background.
The HUD tints it per stage, the same way vanilla tints its white status icons.

usage: python tools/make_icon.py
Needs DayZ Tools (ImageToPAA), numpy and the Windows font Arial Bold (for the Z). The result is committed.
"""
import math
import os
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from build import ROOT, find_app  # noqa: E402

SIZE = 128
SUPER = 4                   # drawn 4x larger, then scaled down for smooth edges
OUT_PNG = os.path.join(ROOT, 'build', 'zv_biohazard_ca.png')
OUT_PAA = os.path.join(ROOT, 'ZombieVirus', 'data', 'zv_biohazard_ca.paa')
FONTS = r'C:\Windows\Fonts'


def fit_glyph(text, font_file, box):
    """Largest font size whose ink bounding box fits in `box` pixels."""
    size = box
    while size > 4:
        font = ImageFont.truetype(os.path.join(FONTS, font_file), size)
        l, t, r, b = font.getbbox(text)
        if r - l <= box and b - t <= box:
            return font, (l, t, r, b)
        size -= 2
    raise RuntimeError('cannot fit ' + text)


def biohazard_mask(n):
    """n x n boolean mask of three heavy biohazard horns, built from circles (units: symbol radius ~= 27).
    No inner ring: at HUD size (35 px) it only adds noise."""
    import numpy as np
    unit = n / 2 / 27.5
    ys, xs = np.mgrid[0:n, 0:n]
    x = (xs - n / 2 + 0.5) / unit
    y = -(ys - n / 2 + 0.5) / unit + 2.5        # shift so the three horns sit centred
    def disk(cx, cy, r):
        return (x - cx) ** 2 + (y - cy) ** 2 <= r * r
    horns = np.zeros((n, n), bool)
    for deg in (90, 210, 330):
        a = math.radians(deg)
        ux, uy = math.cos(a), math.sin(a)
        horns |= disk(ux * 11, uy * 11, 15) & ~disk(ux * 17, uy * 17, 10)
    return horns, unit, 2.5


def draw_centered(draw, text, font, bbox, cx, cy, fill):
    l, t, r, b = bbox
    draw.text((cx - (l + r) / 2, cy - (t + b) / 2), text, font=font, fill=fill)


def main():
    big = SIZE * SUPER
    mask, unit, shift = biohazard_mask(big)
    img = Image.fromarray((mask * 255).astype('uint8'), 'L')
    d = ImageDraw.Draw(img)

    # clear the centre and put a bold Z there
    cx = big / 2
    cy = big / 2 + shift * unit
    hole = 9 * unit
    d.ellipse((cx - hole, cy - hole, cx + hole, cy + hole), fill=0)
    zfont, zbox = fit_glyph('Z', 'arialbd.ttf', int(hole * 1.38))
    draw_centered(d, 'Z', zfont, zbox, cx, cy, 255)

    alpha = img.resize((SIZE, SIZE), Image.LANCZOS)
    out = Image.new('RGBA', (SIZE, SIZE), (255, 255, 255, 0))
    out.putalpha(alpha)
    os.makedirs(os.path.dirname(OUT_PNG), exist_ok=True)
    out.save(OUT_PNG)

    tools = find_app('DayZ Tools', os.path.join('Bin', 'ImageToPAA', 'ImageToPAA.exe'))
    if not tools:
        print('DayZ Tools not found; PNG only: ' + OUT_PNG)
        return 1
    exe = os.path.join(tools, 'Bin', 'ImageToPAA', 'ImageToPAA.exe')
    r = subprocess.run([exe, OUT_PNG, OUT_PAA], capture_output=True, text=True)
    if not os.path.isfile(OUT_PAA):
        print('ImageToPAA failed:\n' + (r.stdout + r.stderr))
        return 1
    print('-> ' + OUT_PAA)
    return 0


if __name__ == '__main__':
    sys.exit(main())
