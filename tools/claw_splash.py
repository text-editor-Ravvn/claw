import sys
import tkinter as tk
from pathlib import Path

IMAGE = Path(__file__).resolve().parent.parent / "assets" / "claw-splash.png"

root = tk.Tk()
root.title("CLAW")
root.configure(bg="black")
root.attributes("-fullscreen", True)

try:
    image = tk.PhotoImage(file=str(IMAGE))
except Exception as exc:
    root.destroy()
    print(f"Could not load CLAW splash image: {exc}")
    sys.exit(1)

screen_w = root.winfo_screenwidth()
screen_h = root.winfo_screenheight()

# Fit the original PNG to the screen without stretching it.
scale = max(
    1,
    (image.width() + screen_w - 1) // screen_w,
    (image.height() + (screen_h - 120) - 1) // (screen_h - 120)
)
if scale > 1:
    image = image.subsample(scale, scale)

content = tk.Frame(root, bg="black")
content.pack(expand=True, fill="both")

picture = tk.Label(content, image=image, bg="black", borderwidth=0)
picture.pack(expand=True, pady=(20, 0))

prompt = tk.Label(
    root,
    text="PRESS SPACE TO CONTINUE",
    font=("Consolas", 18, "bold"),
    fg="#00FFFF",
    bg="black"
)
prompt.pack(side="bottom", pady=30)

def close_splash(event=None):
    root.destroy()

root.bind("<space>", close_splash)
root.bind("<Escape>", close_splash)
root.bind("<Return>", close_splash)
root.focus_force()
root.mainloop()
