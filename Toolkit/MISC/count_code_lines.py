#!/usr/bin/env python3
"""
Visera Engine Code Line Counter
Automatically discovers and counts lines of code across all engine modules.
Usage: python count_code_lines.py [--root /path/to/Visera]
"""
import argparse
import os
from pathlib import Path
from collections import defaultdict
from typing import Dict, Set

# File extensions we consider as code (Engine-wide)
CPP_EXTS = {".ixx", ".hpp", ".h", ".hh", ".hxx", ".cpp", ".cxx", ".cc"}
CMAKE_EXTS = {".cmake", "CMakeLists.txt"}
SHADER_EXTS = {".slang", ".hlsl", ".glsl"}
CSHARP_EXTS = {".cs"}

CATEGORY_EXTS: Dict[str, Set[str]] = {
    "C++": CPP_EXTS,
    "CMake": CMAKE_EXTS,
    "Shader": SHADER_EXTS,
    "C#": CSHARP_EXTS,
}

# Directories to ignore anywhere under Engine
IGNORE_DIR_NAMES = {
    "External",
    ".git",
    ".vs",
    ".idea",
    "Build",
    "Binaries",
    "Intermediate",
    "DerivedDataCache",
    "Cache",
}

SCRIPT_DIR = Path(__file__).parent.resolve()
DEFAULT_PROJECT_ROOT = SCRIPT_DIR.parent.parent.resolve()


def count_lines_in_file(file_path: Path) -> int:
    """Count non-empty lines in a file safely."""
    try:
        with file_path.open("r", encoding="utf-8", errors="ignore") as f:
            return sum(1 for line in f if line.strip())
    except Exception as e:
        print(f"[WARN] Could not read {file_path}: {e}")
        return 0


def _should_ignore_dir(dir_path: Path) -> bool:
    """Ignore only when the current directory name is in the ignore set (not ancestors)."""
    return dir_path.name in IGNORE_DIR_NAMES


def count_lines_in_dir(base_dir: Path, exts: Set[str]) -> int:
    """Recursively count non-empty lines for given extensions in a directory."""
    if not base_dir.exists():
        return 0
    
    total = 0
    for root, dirs, files in os.walk(base_dir):
        root_path = Path(root)
        # Prune ignored dirs for efficiency
        dirs[:] = [d for d in dirs if d not in IGNORE_DIR_NAMES]
        if _should_ignore_dir(root_path):
            continue
            
        for file in files:
            suffix = Path(file).suffix.lower()
            # Handle special case for CMakeLists.txt which has no suffix but is a full filename
            if file in exts or suffix in exts:
                total += count_lines_in_file(Path(root) / file)
    return total


def discover_engine_toplevel_dirs(engine_root: Path) -> Dict[str, Path]:
    """Discover top-level directories under Engine/ (Core, Runtime, Platform, ...)."""
    result: Dict[str, Path] = {}
    if not engine_root.exists():
        return result
    for item in engine_root.iterdir():
        if not item.is_dir() or item.name.startswith(".") or item.name in IGNORE_DIR_NAMES:
            continue
        result[item.name] = item
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description="Count lines of code in Visera Engine.")
    parser.add_argument(
        "--root",
        type=Path,
        default=DEFAULT_PROJECT_ROOT,
        help=f"Visera project root (default: {DEFAULT_PROJECT_ROOT})",
    )
    args = parser.parse_args()
    project_root = args.root.resolve()
    engine_root = project_root / "Engine"

    print("Visera Engine Code Line Counter")
    print("=" * 60)
    print(f"Root: {project_root}")
    print()

    grand_total = 0
    category_totals: Dict[str, int] = defaultdict(int)
    engine_dirs = discover_engine_toplevel_dirs(engine_root)

    if not engine_dirs:
        print(f"[ERROR] Engine root not found at {engine_root}")
        return

    print("Engine/ breakdown (by top-level directory):")
    print("-" * 60)

    for dir_name in sorted(engine_dirs.keys()):
        base_dir = engine_dirs[dir_name]
        per_dir_total = 0
        lines_for_dir: list[tuple[str, int]] = []

        for category in ("C++", "CMake", "Shader", "C#"):
            exts = CATEGORY_EXTS[category]
            total = count_lines_in_dir(base_dir, exts)
            if total <= 0:
                continue
            per_dir_total += total
            grand_total += total
            category_totals[category] += total
            lines_for_dir.append((category, total))

        if not lines_for_dir:
            continue

        rel = base_dir.relative_to(project_root)
        print(f"  [{dir_name}] {rel}")
        for category, count in lines_for_dir:
            print(f"      {category:<8} {count:>8} LOC")
        print(f"      -> subtotal {per_dir_total:>8} LOC")
        print()

    print("=" * 60)
    print("Summary by category:")
    for category in sorted(category_totals.keys()):
        print(f"  {category:<8}: {category_totals[category]:>10} LOC")
    print("=" * 60)
    print(f"ENGINE TOTAL: {grand_total:>10} LOC")
    print()


if __name__ == "__main__":
    main()
