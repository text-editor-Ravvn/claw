"""
Converts assets/claw-splash.png into assets/claw-splash.ansi using real ASCII characters
matching the CLAW branding palette (silver metallic creature + cyan CLAW logo).
"""
import math
import os
import sys

SOURCE = "assets/claw-splash.png"
OUTPUT = "assets/claw-splash.ansi"
COLS = 100
ROWS = 40
RAMP = " .claw"
LETTERS = "CLAW"
LETTER_EDGES = [0.3155, 0.4665, 0.6563]

def main():
    if not os.path.exists(SOURCE):
        print(f"Error: {SOURCE} not found", file=sys.stderr)
        return 1

    try:
        from PIL import Image
    except ImportError:
        print("Pillow not installed, cannot convert image.", file=sys.stderr)
        return 1

    im = Image.open(SOURCE).convert("RGBA")
    bg = Image.new("RGBA", im.size, (0, 0, 0, 255))
    bg.alpha_composite(im)
    im = bg.convert("RGB")

    width, height = im.size
    pixels = im.load()

    xs = [round(x * width / COLS) for x in range(COLS + 1)]
    ys = [round(y * height / ROWS) for y in range(ROWS + 1)]

    density = []
    blue = []
    bright = []

    for y in range(ROWS):
        y0, y1 = ys[y], ys[y + 1]
        if y1 <= y0:
            y1 = y0 + 1
        for x in range(COLS):
            x0, x1 = xs[x], xs[x + 1]
            if x1 <= x0:
                x1 = x0 + 1
            sum_lum = 0
            sum_blue = 0
            for py in range(y0, min(y1, height)):
                for px in range(x0, min(x1, width)):
                    r, g, b_val = pixels[px, py]
                    lum = (r * 299 + g * 587 + b_val * 114) // 1000
                    sum_lum += lum
                    sat = max(0, b_val - max(r, g))
                    sum_blue += lum * sat

            area = (x1 - x0) * (y1 - y0)
            d = sum_lum / (area * 255.0)
            density.append(d)
            b_ratio = (sum_blue / (sum_lum * 255.0)) if sum_lum > 0 else 0.0
            blue.append(b_ratio)
            if d > 0.02:
                bright.append(d)

    bright.sort()
    ref = bright[int(len(bright) * 0.80)] if bright else 1.0
    ref = max(0.05, ref)
    top = len(RAMP) - 2

    lines = []
    for y in range(ROWS):
        sb = []
        last_color = ""
        for x in range(COLS):
            ci = y * COLS + x
            d = density[ci] / ref
            d = min(1.0, max(0.0, d))
            v = math.sqrt(d * math.sqrt(d))

            ch = " "
            t = 0.0
            if v >= 0.07:
                idx = int(v * top + 0.5)
                idx = max(1, min(top, idx))
                ch = RAMP[idx]
                t = min(1.0, blue[ci] * 2.5)
                if t > 0.4:
                    if idx <= 1:
                        ch = "."
                    else:
                        frac = (x + 0.5) / COLS
                        li = sum(frac > edge for edge in LETTER_EDGES)
                        ch = LETTERS[li]

            if ch != " ":
                r_col = int(226 + (40 - 226) * t)
                g_col = int(226 + (160 - 226) * t)
                b_col = int(228 + (255 - 228) * t)
                col_str = f"\033[1;38;2;{r_col};{g_col};{b_col}m" if t > 0.4 else f"\033[38;2;{r_col};{g_col};{b_col}m"
                if col_str != last_color:
                    sb.append(col_str)
                    last_color = col_str
                sb.append(ch)
            else:
                if last_color:
                    sb.append("\033[0m")
                    last_color = ""
                sb.append(" ")

        if last_color:
            sb.append("\033[0m")

        lines.append("".join(sb))

    with open(OUTPUT, "w", encoding="utf-8") as f:
        for l in lines:
            f.write(l + "\n")

    print(f"Created {OUTPUT} with pure ASCII characters ({COLS} columns x {ROWS} rows)")
    return 0

if __name__ == "__main__":
    sys.exit(main())
