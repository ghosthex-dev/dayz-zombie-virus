"""Generate a polished 512x512 Steam Workshop thumbnail for Zombie Virus.

usage: python tools/make_thumbnail.py
"""
import math
import os
import sys
from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_PATH = os.path.join(ROOT, 'build', 'thumbnail_graphic.png')
ICON_PATH = os.path.join(ROOT, 'build', 'zv_biohazard_ca.png')
FONTS = r'C:\Windows\Fonts'

SIZE = 512


def create_graphic_thumbnail():
    # Base dark gritty canvas
    img = Image.new('RGBA', (SIZE, SIZE), (15, 18, 16, 255))
    draw = ImageDraw.Draw(img)

    # 1. Dark radial gradient vignette
    cx, cy = SIZE // 2, SIZE // 2
    for r in range(SIZE, 0, -4):
        alpha = int(255 * (1.0 - (r / SIZE) ** 1.5))
        # subtle toxic green / olive tint in the center
        color = (18 + int(15 * (1 - r / SIZE)), 26 + int(25 * (1 - r / SIZE)), 18 + int(10 * (1 - r / SIZE)), 255)
        draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=color)

    # 2. Subtle grid / technical overlay lines
    grid_overlay = Image.new('RGBA', (SIZE, SIZE), (0, 0, 0, 0))
    gdraw = ImageDraw.Draw(grid_overlay)
    for y in range(0, SIZE, 8):
        gdraw.line([(0, y), (SIZE, y)], fill=(0, 0, 0, 35), width=1)
    img = Image.alpha_composite(img, grid_overlay)

    # 3. Hazard warning stripe bar across top and bottom
    stripe_w = 14
    for x in range(-stripe_w, SIZE + stripe_w * 2, stripe_w * 2):
        pts_top = [(x, 0), (x + stripe_w, 0), (x, 10), (x - stripe_w, 10)]
        draw.polygon(pts_top, fill=(220, 160, 20, 180))
        pts_bot = [(x, SIZE - 10), (x + stripe_w, SIZE - 10), (x, SIZE), (x - stripe_w, SIZE)]
        draw.polygon(pts_bot, fill=(220, 160, 20, 180))

    # 4. Biohazard Symbol with glow
    if os.path.isfile(ICON_PATH):
        raw_icon = Image.open(ICON_PATH).convert('RGBA')
        icon_size = 220
        raw_icon = raw_icon.resize((icon_size, icon_size), Image.LANCZOS)

        # Tint icon to biohazard toxic green/amber
        # The icon has white foreground on alpha
        tinted = Image.new('RGBA', (icon_size, icon_size), (0, 0, 0, 0))
        r_px, g_px, b_px, a_px = raw_icon.split()
        # Glow layer
        glow = Image.new('RGBA', (SIZE, SIZE), (0, 0, 0, 0))
        glow_icon = Image.merge('RGBA', (
            Image.new('L', (icon_size, icon_size), 50),
            Image.new('L', (icon_size, icon_size), 220),
            Image.new('L', (icon_size, icon_size), 80),
            a_px
        ))
        icon_x = (SIZE - icon_size) // 2
        icon_y = 70
        glow.paste(glow_icon, (icon_x, icon_y), a_px)
        glow = glow.filter(ImageFilter.GaussianBlur(radius=18))
        img = Image.alpha_composite(img, glow)

        # Solid icon
        solid_icon = Image.merge('RGBA', (
            Image.new('L', (icon_size, icon_size), 220),
            Image.new('L', (icon_size, icon_size), 245),
            Image.new('L', (icon_size, icon_size), 220),
            a_px
        ))
        img.paste(solid_icon, (icon_x, icon_y), a_px)

    # 5. Bold Typography
    draw = ImageDraw.Draw(img)
    try:
        title_font = ImageFont.truetype(os.path.join(FONTS, 'impact.ttf'), 56)
        sub_font = ImageFont.truetype(os.path.join(FONTS, 'arialbd.ttf'), 18)
        tag_font = ImageFont.truetype(os.path.join(FONTS, 'arialbd.ttf'), 13)
    except IOError:
        title_font = ImageFont.load_default()
        sub_font = ImageFont.load_default()
        tag_font = ImageFont.load_default()

    # Title shadow & text: "ZOMBIE VIRUS"
    title = "ZOMBIE VIRUS"
    t_bbox = draw.textbbox((0, 0), title, font=title_font)
    t_w = t_bbox[2] - t_bbox[0]
    t_x = (SIZE - t_w) // 2
    t_y = 315

    # Title outer glow / shadow
    for ox, oy in [(-2, -2), (2, -2), (-2, 2), (2, 2), (0, 3), (0, -3), (3, 0), (-3, 0)]:
        draw.text((t_x + ox, t_y + oy), title, font=title_font, fill=(0, 0, 0, 240))
    # Green accent under-shadow
    draw.text((t_x, t_y + 3), title, font=title_font, fill=(30, 140, 50, 220))
    # Main title in crisp clean white
    draw.text((t_x, t_y), title, font=title_font, fill=(245, 250, 245, 255))

    # Subtitle: "Z-STRAIN INFECTION • ANTIVIRAL CURE"
    subtitle = "Z-STRAIN INFECTION • RISE AS THE INFECTED"
    s_bbox = draw.textbbox((0, 0), subtitle, font=sub_font)
    s_w = s_bbox[2] - s_bbox[0]
    s_x = (SIZE - s_w) // 2
    s_y = 385

    # Subtitle pill box
    draw.rectangle([s_x - 12, s_y - 4, s_x + s_w + 12, s_y + (s_bbox[3] - s_bbox[1]) + 4], fill=(0, 0, 0, 180), outline=(50, 160, 70, 180), width=1)
    draw.text((s_x, s_y), subtitle, font=sub_font, fill=(180, 235, 190, 255))

    # Tagline / Author: "A DAYZ STANDALONE MOD • GHOSTHEX"
    tag = "VANILLA-FRIENDLY MULTIPLAYER MOD  |  BY GHOSTHEX"
    tag_bbox = draw.textbbox((0, 0), tag, font=tag_font)
    tag_w = tag_bbox[2] - tag_bbox[0]
    tag_x = (SIZE - tag_w) // 2
    tag_y = 445
    draw.text((tag_x, tag_y), tag, font=tag_font, fill=(160, 175, 165, 220))

    # 6. Subtle border frame
    draw.rectangle([6, 6, SIZE - 7, SIZE - 7], outline=(60, 80, 65, 160), width=2)
    # Corner brackets
    corner_len = 24
    c_col = (180, 220, 100, 220)
    for ox, oy, dx, dy in [(12, 12, 1, 1), (SIZE - 12, 12, -1, 1), (12, SIZE - 12, 1, -1), (SIZE - 12, SIZE - 12, -1, -1)]:
        draw.line([(ox, oy), (ox + dx * corner_len, oy)], fill=c_col, width=2)
        draw.line([(ox, oy), (ox, oy + dy * corner_len)], fill=c_col, width=2)

    os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
    img.save(OUT_PATH)
    print("Generated graphic thumbnail:", OUT_PATH)
    return OUT_PATH


if __name__ == '__main__':
    create_graphic_thumbnail()
