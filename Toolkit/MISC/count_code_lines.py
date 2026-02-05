#!/usr/bin/env python3
"""
Visera Engine Code Line Counter
Automatically discovers and counts lines of code across all engine modules.
"""
import os
from pathlib import Path
from collections import defaultdict
from typing import Dict, Iterable, Set, Tuple

# File extensions we consider as code (Engine-wide)
# Note: keep these conservative; you can extend as needed.
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
    "External",  # third-party
    ".git",
    ".vs",
    ".idea",
    "Build",
    "Binaries",
    "Intermediate",
    "DerivedDataCache",
    "Cache",
}

# Base directory (script location's parent's parent = project root)
SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_ROOT = SCRIPT_DIR.parent.parent.resolve()
ENGINE_ROOT = PROJECT_ROOT / "Engine"


def count_lines_in_file(file_path: Path) -> int:
    """Count non-empty lines in a file safely."""
    try:
        with file_path.open("r", encoding="utf-8", errors="ignore") as f:
            return sum(1 for line in f if line.strip())
    except Exception as e:
        print(f"[WARN] Could not read {file_path}: {e}")
        return 0


def _should_ignore_dir(dir_path: Path) -> bool:
    return any(part in IGNORE_DIR_NAMES for part in dir_path.parts)


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


def discover_engine_toplevel_dirs() -> Dict[str, Path]:
    """Discover top-level directories under Engine/ (Core, Runtime, Platform, API, ...)."""
    result: Dict[str, Path] = {}
    if not ENGINE_ROOT.exists():
        return result

    for item in ENGINE_ROOT.iterdir():
        if not item.is_dir():
            continue
        if item.name.startswith("."):
            continue
        if item.name in IGNORE_DIR_NAMES:
            continue
        result[item.name] = item

    return result


def format_output(category: str, name: str, count: int, path: Path) -> str:
    """Format a single output line."""
    rel_path = path.relative_to(PROJECT_ROOT)
    return f"{count:<10} LOC ({category:<8}) | {rel_path}"


def main():
    """Main entry point."""
    print("Visera Engine Code Line Counter")
    print("=" * 60)
    print()
    
    grand_total = 0
    category_totals = defaultdict(int)

    engine_dirs = discover_engine_toplevel_dirs()
    if not engine_dirs:
        print(f"[ERROR] Engine root not found at {ENGINE_ROOT}")
        return

    # Print per top-level dir and per category
    print("Engine/ Breakdown (by top-level directory):")
    print("-" * 60)

    # Keep output stable
    for dir_name in sorted(engine_dirs.keys()):
        base_dir = engine_dirs[dir_name]
        per_dir_total = 0

        for category in ("C++", "CMake", "Shader", "C#"):
            exts = CATEGORY_EXTS[category]
            total = count_lines_in_dir(base_dir, exts)
            if total <= 0:
                continue
            per_dir_total += total
            grand_total += total
            category_totals[category] += total
            print(format_output(category, dir_name, total, base_dir))

        # If a directory has no recognized code files, it's omitted.

    print()
    print("=" * 60)
    print("Summary by Category:")
    for category in sorted(category_totals.keys()):
        print(f"  {category:<8}: {category_totals[category]:>10} LOC")
    print("=" * 60)
    print(f"ENGINE TOTAL: {grand_total:>10} LOC")
    print()


if __name__ == "__main__":
    main()
