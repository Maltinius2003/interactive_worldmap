from PIL import Image
import numpy as np
import os
from tkinter import Tk
from tkinter.filedialog import askopenfilename

# Popup-Fenster zur Auswahl der Bilddatei
Tk().withdraw()  # Versteckt das Hauptfenster
image_path = askopenfilename(title="Select an image file", filetypes=[("Image files", "*.png;*.jpg;*.jpeg;*.bmp")])

if not image_path:
    raise FileNotFoundError("No file was selected.")

# Bild einlesen und in Graustufen umwandeln
image = Image.open(image_path).convert("L")  # "L" = Luminanz (Graustufen)
image = image.resize((210, 210))  # sicherstellen, dass Bild 210x210 ist

# Bilddaten als NumPy-Array
gray_array = np.array(image)

# Schwellwert (z. B. 128)
threshold = 128
bool_matrix = gray_array < threshold

# Matrix als Textdatei speichern
original_name = os.path.splitext(os.path.basename(image_path))[0]
script_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(script_dir, f"{original_name}_MATRIX.txt")
np.savetxt(output_path, bool_matrix.astype(int), fmt='%d')

# Matrix für Arduino-Code formatieren (auf dem Kopf), 0,0 ist unten
arduino_matrix = np.flipud(bool_matrix)
arduino_code = ""

for row_idx, row in enumerate(arduino_matrix):
    for col_idx, val in enumerate(row):
        if val:
            arduino_code += f"led_matrix[{row_idx}][{col_idx}] = true;\n"

arduino_output_path = os.path.join(script_dir, f"{original_name}_ARDUINO_CODE.txt")
with open(arduino_output_path, "w") as f:
    f.write(arduino_code)

print(f"Matrix saved to {output_path}")
print(f"Arduino matrix saved to {arduino_output_path}")
