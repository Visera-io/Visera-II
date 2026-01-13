#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Generate GLFW window icons (RGBA) in multiple sizes and embed them into a C++ header.
- Input: any image format Pillow can read (png/jpg/webp/...)
- Output: a single .hpp with embedded uint8_t arrays and a helper function.

Usage:
  python gen_glfw_icon_embed.py input.png out/ViseraWindowIcon.hpp --name ViseraWindowIcon

Then in C++:
  #include "ViseraWindowIcon.hpp"
  glfwSetWindowIcon(window, ViseraWindowIcon::GetCount(), ViseraWindowIcon::GetImages());
"""

import argparse
from pathlib import Path
from typing import List, Tuple

from PIL import Image


DEFAULT_SIZES = [16, 32, 48, 64, 128, 256]


def sanitize_cpp_identifier(name: str) -> str:
    # very small sanitizer: keep alnum and underscores, no leading digit
    out = []
    for c in name:
        if c.isalnum() or c == "_":
            out.append(c)
        else:
            out.append("_")
    if not out:
        return "EmbeddedIcon"
    if out[0].isdigit():
        out.insert(0, "_")
    return "".join(out)


def to_rgba_square(img: Image.Image, size: int) -> Image.Image:
    """
    Convert to RGBA and resize to square with padding (keeps aspect ratio).
    This avoids distortion and guarantees size x size.
    """
    img = img.convert("RGBA")

    # Compute scale to fit within size
    w, h = img.size
    if w == 0 or h == 0:
        raise ValueError("Invalid image size (0).")

    scale = min(size / w, size / h)
    new_w = max(1, int(round(w * scale)))
    new_h = max(1, int(round(h * scale)))

    resized = img.resize((new_w, new_h), resample=Image.Resampling.LANCZOS)

    # Paste centered into a transparent square canvas
    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    x = (size - new_w) // 2
    y = (size - new_h) // 2
    canvas.paste(resized, (x, y))
    return canvas


def bytes_to_cpp_array(data: bytes, indent: str = "    ", cols: int = 16) -> str:
    """
    Format bytes as C++ initializer list: { 0x00, 0x01, ... }
    """
    hexes = [f"0x{b:02X}" for b in data]
    lines = []
    for i in range(0, len(hexes), cols):
        chunk = ", ".join(hexes[i:i + cols])
        lines.append(f"{indent}{chunk},")
    return "\n".join(lines)


def generate_cpp_header(namespace: str,
                        sizes_and_rgba: List[Tuple[int, bytes]],
                        header_guard: str) -> str:
    """
    Produces a C++ header with embedded RGBA arrays and GLFWimage descriptors.
    """
    ns = sanitize_cpp_identifier(namespace)
    guard = sanitize_cpp_identifier(header_guard).upper()

    # Generate arrays
    arrays = []
    image_entries = []
    for size, rgba in sizes_and_rgba:
        arr_name = f"kIconRGBA_{size}"
        arrays.append(
            f"inline constexpr unsigned char {arr_name}[] = {{\n"
            f"{bytes_to_cpp_array(rgba)}\n"
            f"}};\n"
        )
        image_entries.append(
            f"    {{ {size}, {size}, const_cast<unsigned char*>({arr_name}) }},"
        )

    arrays_blob = "\n".join(arrays)

    # Note: GLFWimage expects (int width, int height, unsigned char* pixels).
    # We use const_cast to satisfy GLFW's non-const pointer API.
    images_blob = "\n".join(image_entries)

    return f"""#pragma once
#ifndef {guard}
#define {guard}

#include <cstddef>
#include <GLFW/glfw3.h>

namespace {ns}
{{
{arrays_blob}
inline GLFWimage kImages[] = {{
{images_blob}
}};

inline constexpr int GetCount() noexcept
{{
    return static_cast<int>(sizeof(kImages) / sizeof(kImages[0]));
}}

inline GLFWimage* GetImages() noexcept
{{
    return kImages;
}}
}} // namespace {ns}

#endif // {guard}
"""


def main():
    ap = argparse.ArgumentParser(description="Generate embedded GLFW window icons (RGBA) in multiple sizes.")
    ap.add_argument("input_image", type=str, help="Path to input image (png/jpg/webp/...)")
    ap.add_argument("output_hpp", type=str, help="Path to output .hpp")
    ap.add_argument("--name", type=str, default="ViseraWindowIcon", help="C++ namespace/name (default: ViseraWindowIcon)")
    ap.add_argument("--sizes", type=str, default=",".join(map(str, DEFAULT_SIZES)),
                    help=f"Comma-separated sizes (default: {DEFAULT_SIZES})")
    ap.add_argument("--no-pad", action="store_true",
                    help="If set, will stretch image to square (no aspect preserve). Not recommended.")
    args = ap.parse_args()

    in_path = Path(args.input_image)
    out_path = Path(args.output_hpp)
    out_path.parent.mkdir(parents=True, exist_ok=True)

    sizes = []
    for part in args.sizes.split(","):
        part = part.strip()
        if not part:
            continue
        n = int(part)
        if n <= 0:
            raise ValueError(f"Invalid size: {n}")
        sizes.append(n)

    if not sizes:
        raise ValueError("No sizes specified.")

    img = Image.open(in_path)

    sizes_and_rgba: List[Tuple[int, bytes]] = []
    for s in sizes:
        if args.no_pad:
            icon = img.convert("RGBA").resize((s, s), resample=Image.Resampling.LANCZOS)
        else:
            icon = to_rgba_square(img, s)

        rgba = icon.tobytes("raw", "RGBA")
        expected = s * s * 4
        if len(rgba) != expected:
            raise RuntimeError(f"RGBA size mismatch for {s}x{s}: got {len(rgba)} expected {expected}")
        sizes_and_rgba.append((s, rgba))

    guard = f"{args.name}_HPP"
    header_text = generate_cpp_header(args.name, sizes_and_rgba, guard)
    out_path.write_text(header_text, encoding="utf-8")

    print(f"[OK] Wrote: {out_path}")
    print(f"     Sizes: {sizes}")
    print(f"     Namespace: {sanitize_cpp_identifier(args.name)}")


if __name__ == "__main__":
    main()
