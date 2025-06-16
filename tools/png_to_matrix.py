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
image = Image.open(image_path).convert("L")
image = image.resize((210, 210))

# Grauwert-Array und binäre Schwelle
gray_array = np.array(image)
threshold = 128
bool_matrix = gray_array < threshold

# Dateinamen vorbereiten
original_name = os.path.splitext(os.path.basename(image_path))[0]
script_dir = os.path.dirname(os.path.abspath(__file__))

# === 1. NORMALE MATRIX ===

# Mittleren Ausschnitt nehmen: Zeilen 1 bis 208 (Index 1 bis 208 exklusiv 209)
cropped_matrix = bool_matrix[1:209, :]  # shape = (208, 210)

# Speichern als einfache Matrix-Datei (mittlerer Ausschnitt)
matrix_path = os.path.join(script_dir, f"{original_name}_MATRIX.txt")
np.savetxt(matrix_path, cropped_matrix.astype(int), fmt='%d')

# Arduino-Code-Zuweisungen (x/y vertauscht, mittlerer Ausschnitt)
arduino_code = ""
for row_idx, row in enumerate(cropped_matrix):
    for col_idx, val in enumerate(row):
        if val:
            arduino_code += f"led_matrix_blue[{col_idx}][{row_idx}] = true;\n"

arduino_code_path = os.path.join(script_dir, f"{original_name}_ARDUINO_CODE.txt")
with open(arduino_code_path, "w") as f:
    f.write(arduino_code)

# === 2. GESPIEGELTE MATRIX ===

# Flip auf gesamtes Original anwenden und danach ebenfalls mittleren Bereich ausschneiden
flipped_matrix = np.flipud(bool_matrix)[1:209, :]

flipped_matrix_path = os.path.join(script_dir, f"{original_name}_MATRIX_FLIPPED.txt")
np.savetxt(flipped_matrix_path, flipped_matrix.astype(int), fmt='%d')

flipped_code = ""
for row_idx, row in enumerate(flipped_matrix):
    for col_idx, val in enumerate(row):
        if val:
            flipped_code += f"led_matrix_blue[{col_idx}][{row_idx}] = true;\n"

flipped_code_path = os.path.join(script_dir, f"{original_name}_ARDUINO_CODE_FLIPPED.txt")
with open(flipped_code_path, "w") as f:
    f.write(flipped_code)

# === Meldung ===
print(f"Normale Matrix gespeichert unter:\n{matrix_path}")
print(f"Normale Arduino-Zuweisungen unter:\n{arduino_code_path}")
print(f"Flip-Matrix gespeichert unter:\n{flipped_matrix_path}")
print(f"Flip-Arduino-Zuweisungen unter:\n{flipped_code_path}")
