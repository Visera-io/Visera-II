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

# File extensions we consider as code
CODE_EXTS = {".hpp", ".ixx"}

def count_lines_in_file(file_path: Path) -> int:
    """Count non-empty lines in a file safely."""
    try:
        with file_path.open("r", encoding="utf-8", errors="ignore") as f:
            return sum(1 for line in f if line.strip())
    except Exception as e:
        print(f"⚠️ Could not read {file_path}: {e}")
        return 0

def count_lines_in_dir(base_dir: Path) -> int:
    """Recursively count lines of code in a directory."""
    total = 0
    for root, _, files in os.walk(base_dir):
        for file in files:
            if Path(file).suffix in CODE_EXTS:
                total += count_lines_in_file(Path(root) / file)
    return total

def main():
    grand_total = 0
    for folder in SOURCE_DIRS:
        path = Path(folder)
        if path.exists():
            total = count_lines_in_dir(path)
            grand_total += total
            print(f"{folder}: {total} LOC")
        else:
            print(f"⚠️ Skipping missing folder: {folder}")
    print("=" * 40)
    print(f"TOTAL: {grand_total} LOC")

if __name__ == "__main__":
    main()