from PIL import Image, ImageEnhance, ImageFilter

SOURCE = "assets/claw-splash.png"
OUTPUT = "assets/claw-splash.ansi"
WIDTH = 120

im = Image.open(SOURCE).convert("RGBA")

# Preserve transparency against a pure black background.
bg = Image.new("RGBA", im.size, (0, 0, 0, 255))
bg.alpha_composite(im)
im = bg.convert("RGB")

# Improve contrast and sharpen the original artwork before resizing.
im = ImageEnhance.Contrast(im).enhance(1.45)
im = ImageEnhance.Sharpness(im).enhance(2.0)
im = im.filter(ImageFilter.UnsharpMask(
    radius=2, percent=180, threshold=3
))

# Keep the same overall terminal width.
height = round(im.height * WIDTH / im.width)
height += height % 2
im = im.resize((WIDTH, height), Image.Resampling.LANCZOS)

# Sharpen once more after resizing to help terminal edges.
im = im.filter(ImageFilter.UnsharpMask(
    radius=1, percent=120, threshold=2
))

pixels = im.load()

with open(OUTPUT, "w", encoding="utf-8") as f:
    for y in range(0, height, 2):
        for x in range(WIDTH):
            r1, g1, b1 = pixels[x, y]
            r2, g2, b2 = pixels[x, y + 1]

            f.write(
                f"\033[38;2;{r1};{g1};{b1}m"
                f"\033[48;2;{r2};{g2};{b2}m▀"
            )

        f.write("\033[0m\n")

print(f"Created {OUTPUT}: {WIDTH} columns x {height // 2} rows")
