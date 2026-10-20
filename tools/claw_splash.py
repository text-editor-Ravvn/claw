import ctypes
import os
import sys
import tkinter as tk
from pathlib import Path

# Enable Per-Monitor DPI Awareness on Windows so the OS does not bitmap-scale
# and blur the splash window on 125%, 150%, or 200% displays.
if sys.platform == "win32":
    try:
        ctypes.windll.shcore.SetProcessDpiAwareness(2)
    except Exception:
        try:
            ctypes.windll.user32.SetProcessDPIAware()
        except Exception:
            pass

if len(sys.argv) > 1 and Path(sys.argv[1]).is_file():
    IMAGE = Path(sys.argv[1]).resolve()
else:
    IMAGE = Path(__file__).resolve().parent.parent / "assets" / "claw-splash.png"

root = tk.Tk()
root.title("CLAW")
root.configure(bg="black")

try:
    root.attributes("-fullscreen", True)
except Exception:
    root.state("zoomed")

root.update_idletasks()
screen_w = max(640, root.winfo_screenwidth())
screen_h = max(480, root.winfo_screenheight())

image = None
try:
    # Use Pillow if available for pristine color and resolution handling
    from PIL import Image, ImageTk, ImageEnhance, ImageFilter
    pil_img = Image.open(str(IMAGE))
    img_w, img_h = pil_img.size

    # On displays where the image fits (1080p, 1440p, 4K, etc.):
    # Render at 100% 1:1 scale with ZERO resampling for 100% identical, razor-sharp graphics.
    if screen_w >= img_w and screen_h >= img_h:
        image = ImageTk.PhotoImage(pil_img)
    else:
        # For smaller screens (e.g. 768p), fit with high-quality Lanczos and enhance
        # contrast/sharpness so 1-pixel font strokes remain crisp and well-defined.
        scale = min(screen_w / img_w, screen_h / img_h)
        new_w = max(1, int(round(img_w * scale)))
        new_h = max(1, int(round(img_h * scale)))
        resampled = pil_img.resize((new_w, new_h), Image.Resampling.LANCZOS)
        
        # Restore stark black-and-white contrast on font edges
        enhancer = ImageEnhance.Contrast(resampled)
        resampled = enhancer.enhance(1.4)
        sharpener = ImageEnhance.Sharpness(resampled)
        resampled = sharpener.enhance(1.8)
        image = ImageTk.PhotoImage(resampled)
except Exception:
    # Pure Tkinter fallback: load original PNG directly at 100% scale (never subsample!)
    try:
        image = tk.PhotoImage(file=str(IMAGE))
    except Exception as exc:
        root.destroy()
        print(f"Could not load CLAW splash image: {exc}")
        sys.exit(1)

# Fullscreen black canvas to host the image pixel-perfect without layout padding shrink
canvas = tk.Canvas(root, bg="black", highlightthickness=0)
canvas.pack(fill="both", expand=True)

# Center the artwork 1:1
canvas.create_image(screen_w // 2, screen_h // 2, anchor="center", image=image)

# Overlay the prompt cleanly at the bottom without taking vertical space from the image
prompt_y = max(screen_h - 40, screen_h // 2 + 1070 // 2 + 20)
if prompt_y > screen_h - 20:
    prompt_y = screen_h - 25

canvas.create_text(
    screen_w // 2,
    prompt_y,
    text="PRESS SPACE TO CONTINUE",
    font=("Consolas", 18, "bold"),
    fill="#00FFFF"
)

def close_splash(event=None):
    root.destroy()
    sys.exit(0)

root.bind("<space>", close_splash)
root.bind("<Escape>", close_splash)
root.bind("<Return>", close_splash)
root.bind("<KP_Enter>", close_splash)
root.bind("<Key>", close_splash)
root.bind("<Button-1>", close_splash)
root.bind("<Button-2>", close_splash)
root.bind("<Button-3>", close_splash)
root.protocol("WM_DELETE_WINDOW", close_splash)

root.lift()
root.focus_force()
root.mainloop()
