import numpy as np
import os
from tkinter import Tk
from tkinter.filedialog import askopenfilename

# Datei auswählen
Tk().withdraw()
matrix_path = askopenfilename(title="Select a *_MATRIX.txt file", filetypes=[("Text files", "*.txt")])

if not matrix_path:
    raise FileNotFoundError("No file was selected.")

# Matrix einlesen
bool_matrix = np.loadtxt(matrix_path, dtype=int)

# Pfade
original_name = os.path.splitext(os.path.basename(matrix_path))[0].replace("_MATRIX", "")
script_dir = os.path.dirname(os.path.abspath(matrix_path))
output_path = os.path.join(script_dir, f"{original_name}_ARDUINO_CODE.txt")

# Arduino-Zuweisungen generieren (x = Spalte, y = Zeile)
arduino_code = ""
for y, row in enumerate(bool_matrix):
    for x, val in enumerate(row):
        if val:
            arduino_code += f"led_matrix_blue[{x}][{y}] = true;\n"

# Datei schreiben
with open(output_path, "w") as f:
    f.write(arduino_code)

print(f"ARDUINO_CODE geschrieben nach: {output_path}")
