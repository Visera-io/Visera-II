#!/usr/bin/env python3
import os
from pathlib import Path

# Directories to scan
SOURCE_DIRS = [
    "Core/Source",
    "Runtime/Source",
    "Engine/Source",
    "Studio/Source",
]

SCRIPT_DIRS = [
    "Core/Scripts",
    "Runtime/Scripts",
    "Engine/Scripts",
    "Studio/Scripts",
]

SHADER_DIRS = [
    "Apps/AlohaVisera/Assets/Shader",
    "Engine/Assets/Shader",
]

CSHARP_DIRS = [
    "Apps/AlohaVisera/Assets/Script",
    "Engine/APIs",
]

# File extensions we consider as code
CODE_EXTS = {".hpp", ".ixx"}

# File extensions we consider as CMake
CMAKE_EXTS = {".cmake", "CMakeLists.txt"}

def count_lines_in_file(file_path: Path) -> int:
    """Count non-empty lines in a file safely."""
    try:
        with file_path.open("r", encoding="utf-8", errors="ignore") as f:
            return sum(1 for line in f if line.strip())
    except Exception as e:
        print(f"⚠️ Could not read {file_path}: {e}")
        return 0

def count_lines_in_dir(base_dir: Path, exts) -> int:
    """Recursively count lines of code in a directory for given extensions."""
    total = 0
    for root, _, files in os.walk(base_dir):
        for file in files:
            suffix = Path(file).suffix
            # Handle special case for CMakeLists.txt which has no suffix but is a full filename
            if file in exts or suffix in exts:
                total += count_lines_in_file(Path(root) / file)
    return total

def main():
    grand_total = 0
    for folder in SOURCE_DIRS:
        path = Path(folder)
        if path.exists():
            total = count_lines_in_dir(path, CODE_EXTS)
            grand_total += total
            print(f"{total:<10} LOC (C++)\t | {folder}")
        else:
            print(f"⚠️ Skipping missing folder: {folder}")
    for folder in SCRIPT_DIRS:
        path = Path(folder)
        if path.exists():
            total = count_lines_in_dir(path, CMAKE_EXTS)
            grand_total += total
            print(f"{total:<10} LOC (CMake)\t | {folder}")
        else:
            print(f"⚠️ Skipping missing folder: {folder}")

    for folder in SHADER_DIRS:
        path = Path(folder)
        if path.exists():
            total = count_lines_in_dir(path, ".slang")
            grand_total += total
            print(f"{total:<10} LOC (Slang)\t | {folder}")
        else:
            print(f"⚠️ Skipping missing folder: {folder}")

    for folder in CSHARP_DIRS:
        path = Path(folder)
        if path.exists():
            total = count_lines_in_dir(path, ".cs")
            grand_total += total
            print(f"{total:<10} LOC (C#)\t | {folder}")
        else:
            print(f"⚠️ Skipping missing folder: {folder}")

    print("=" * 40)
    print(f"TOTAL: {grand_total} LOC")

if __name__ == "__main__":
    main()