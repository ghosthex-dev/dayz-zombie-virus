"""Make ZombieVirus/data/zv_antiviral_co.paa: the vanilla antidote injector texture, recoloured green.

usage: python tools/make_texture.py
Needs the local DayZ install (source texture), DayZ Tools (ImageToPAA) and the PAA decoder from
C:\\Dev\\dayz-studio\\tools (paa.py + odol_reader.py). The result is committed, so this only runs when the look changes.
"""
import colorsys
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from build import ROOT, find_app, index_game_files  # noqa: E402

sys.path.insert(0, r'C:\Dev\dayz-studio\tools')
import paa  # noqa: E402

SOURCE = r'dz\gear\medical\data\antidote_co.paa'
OUT_PNG = os.path.join(ROOT, 'build', 'zv_antiviral_co.png')
OUT_PAA = os.path.join(ROOT, 'ZombieVirus', 'data', 'zv_antiviral_co.paa')
TARGET_HUE = 0.33      # green


def main():
    game = find_app('DayZ', 'DayZ_x64.exe')
    tools = find_app('DayZ Tools', os.path.join('Bin', 'ImageToPAA', 'ImageToPAA.exe'))
    if not game or not tools:
        print('needs DayZ and DayZ Tools')
        return 2
    idx = index_game_files(game, ('.paa',))
    if SOURCE not in idx:
        print('source texture not found: ' + SOURCE)
        return 1
    pbo, off, size = idx[SOURCE]
    with open(pbo, 'rb') as f:
        f.seek(off)
        img = paa.decode(f.read(size), max_dim=4096)
    if img is None:
        print('cannot decode ' + SOURCE)
        return 1

    # Strongly coloured pixels (the label) are rotated to green; greys, black and white stay as they are.
    px = img.load()
    for y in range(img.height):
        for x in range(img.width):
            r, g, b, a = px[x, y]
            h, s, v = colorsys.rgb_to_hsv(r / 255.0, g / 255.0, b / 255.0)
            if s > 0.25:
                nr, ng, nb = colorsys.hsv_to_rgb(TARGET_HUE, s * 0.75, v * 0.6)
                px[x, y] = (int(nr * 255), int(ng * 255), int(nb * 255), a)

    os.makedirs(os.path.dirname(OUT_PNG), exist_ok=True)
    os.makedirs(os.path.dirname(OUT_PAA), exist_ok=True)
    img.save(OUT_PNG)
    exe = os.path.join(tools, 'Bin', 'ImageToPAA', 'ImageToPAA.exe')
    r = subprocess.run([exe, OUT_PNG, OUT_PAA], capture_output=True, text=True)
    if not os.path.isfile(OUT_PAA):
        print('ImageToPAA failed:\n' + (r.stdout + r.stderr))
        return 1
    print('%dx%d -> %s' % (img.width, img.height, OUT_PAA))
    return 0


if __name__ == '__main__':
    sys.exit(main())
