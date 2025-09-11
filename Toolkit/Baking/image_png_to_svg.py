# image_png_to_svg.py
import sys
import cv2
import numpy as np


def png_to_svg(input_path: str, output_path: str, threshold: int = 128):
    """
    Convert PNG to SVG using contour detection (no potrace required).

    :param input_path: Path to PNG image
    :param output_path: Path to SVG file
    :param threshold: Binarization threshold (0-255)
    """
    # Load image as grayscale
    img = cv2.imread(input_path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise FileNotFoundError(f"Could not load image: {input_path}")

    # Binarize
    _, bw = cv2.threshold(img, threshold, 255, cv2.THRESH_BINARY)

    # Find contours
    contours, _ = cv2.findContours(bw, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # Write SVG
    with open(output_path, "w") as f:
        h, w = bw.shape
        f.write('<?xml version="1.0" standalone="no"?>\n')
        f.write(f'<svg xmlns="http://www.w3.org/2000/svg" '
                f'version="1.1" width="{w}" height="{h}" '
                f'viewBox="0 0 {w} {h}">\n')

        for contour in contours:
            path_data = []
            for i, point in enumerate(contour):
                x, y = point[0]
                if i == 0:
                    path_data.append(f"M{x},{y}")
                else:
                    path_data.append(f"L{x},{y}")
            path_data.append("Z")  # close path

            f.write(f'<path d="{" ".join(path_data)}" fill="black" />\n')

        f.write("</svg>\n")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python image_png_to_svg.py input.png output.svg")
        sys.exit(1)

    png_to_svg(sys.argv[1], sys.argv[2])