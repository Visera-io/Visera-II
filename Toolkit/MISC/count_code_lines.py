#!/usr/bin/env python3
"""
Visera Engine Code Line Counter
Automatically discovers and counts lines of code across all engine modules.
"""
import os
from pathlib import Path
from collections import defaultdict
from typing import Dict, Set, Tuple

# File extensions we consider as code
CODE_EXTS = {".hpp", ".ixx"}
CMAKE_EXTS = {".cmake", "CMakeLists.txt"}
SHADER_EXTS = {".slang", ".hlsl", ".glsl"}
CSHARP_EXTS = {".cs"}

# Base directory (script location's parent's parent = project root)
SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_ROOT = SCRIPT_DIR.parent.parent.resolve()


def count_lines_in_file(file_path: Path) -> int:
    """Count non-empty lines in a file safely."""
    try:
        with file_path.open("r", encoding="utf-8", errors="ignore") as f:
            return sum(1 for line in f if line.strip())
    except Exception as e:
        print(f"⚠️ Could not read {file_path}: {e}")
        return 0


def count_lines_in_dir(base_dir: Path, exts: Set[str]) -> int:
    """Recursively count lines of code in a directory for given extensions."""
    if not base_dir.exists():
        return 0
    
    total = 0
    for root, _, files in os.walk(base_dir):
        # Skip External directories (third-party code)
        if "External" in Path(root).parts:
            continue
            
        for file in files:
            suffix = Path(file).suffix
            # Handle special case for CMakeLists.txt which has no suffix but is a full filename
            if file in exts or suffix in exts:
                total += count_lines_in_file(Path(root) / file)
    return total


def discover_runtime_modules() -> Dict[str, Path]:
    """Automatically discover all Runtime modules."""
    modules = {}
    runtime_dir = PROJECT_ROOT / "Engine" / "Runtime"
    
    if not runtime_dir.exists():
        return modules
    
    for item in runtime_dir.iterdir():
        if item.is_dir() and not item.name.startswith("."):
            source_dir = item / "Source"
            if source_dir.exists():
                modules[item.name] = source_dir
    
    return modules


def discover_core_modules() -> Dict[str, Path]:
    """Discover Core module."""
    modules = {}
    core_dir = PROJECT_ROOT / "Engine" / "Core"
    
    if core_dir.exists():
        source_dir = core_dir / "Source"
        if source_dir.exists():
            modules["Core"] = source_dir
    
    return modules


def discover_api_modules() -> Dict[str, Path]:
    """Discover API module."""
    modules = {}
    api_dir = PROJECT_ROOT / "Engine" / "API"
    
    if api_dir.exists():
        source_dir = api_dir / "Source"
        if source_dir.exists():
            modules["API"] = source_dir
    
    return modules


def discover_script_dirs() -> Dict[str, Path]:
    """Automatically discover all Scripts directories."""
    scripts = {}
    
    # Core Scripts
    core_scripts = PROJECT_ROOT / "Engine" / "Core" / "Scripts"
    if core_scripts.exists():
        scripts["Core"] = core_scripts
    
    # Runtime Scripts
    runtime_dir = PROJECT_ROOT / "Engine" / "Runtime"
    if runtime_dir.exists():
        for item in runtime_dir.iterdir():
            if item.is_dir() and not item.name.startswith("."):
                script_dir = item / "Scripts"
                if script_dir.exists():
                    scripts[item.name] = script_dir
    
    return scripts


def discover_shader_dirs() -> Dict[str, Path]:
    """Automatically discover all Shader directories."""
    shaders = {}
    
    # Engine Shaders
    engine_shaders = PROJECT_ROOT / "Engine" / "Shaders"
    if engine_shaders.exists():
        shaders["Engine/Shaders"] = engine_shaders
    
    # Engine Assets Shader
    engine_assets_shader = PROJECT_ROOT / "Engine" / "Assets" / "Shader"
    if engine_assets_shader.exists():
        shaders["Engine/Assets/Shader"] = engine_assets_shader
    
    # Demo/App Shaders
    demos_dir = PROJECT_ROOT / "Demos"
    if demos_dir.exists():
        for demo_dir in demos_dir.iterdir():
            if demo_dir.is_dir():
                shader_dir = demo_dir / "Assets" / "Shader"
                if shader_dir.exists():
                    shaders[f"Demos/{demo_dir.name}/Assets/Shader"] = shader_dir
    
    return shaders


def discover_csharp_dirs() -> Dict[str, Path]:
    """Automatically discover all C# directories."""
    csharp_dirs = {}
    
    # Engine API C#
    api_csharp = PROJECT_ROOT / "Engine" / "API" / "CSharp"
    if api_csharp.exists():
        csharp_dirs["Engine/API/CSharp"] = api_csharp
    
    # Demo/App Scripts
    demos_dir = PROJECT_ROOT / "Demos"
    if demos_dir.exists():
        for demo_dir in demos_dir.iterdir():
            if demo_dir.is_dir():
                script_dir = demo_dir / "Assets" / "Script"
                if script_dir.exists():
                    csharp_dirs[f"Demos/{demo_dir.name}/Assets/Script"] = script_dir
    
    return csharp_dirs


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
    
    # Discover and count C++ source code
    print("C++ Source Code:")
    print("-" * 60)
    
    all_modules = {}
    all_modules.update(discover_core_modules())
    all_modules.update(discover_runtime_modules())
    all_modules.update(discover_api_modules())
    
    # Sort modules for consistent output
    for module_name in sorted(all_modules.keys()):
        source_dir = all_modules[module_name]
        total = count_lines_in_dir(source_dir, CODE_EXTS)
        if total > 0:
            grand_total += total
            category_totals["C++"] += total
            print(format_output("C++", module_name, total, source_dir))
    
    print()
    
    # Discover and count CMake scripts
    print("CMake Scripts:")
    print("-" * 60)
    
    script_dirs = discover_script_dirs()
    for script_name in sorted(script_dirs.keys()):
        script_dir = script_dirs[script_name]
        total = count_lines_in_dir(script_dir, CMAKE_EXTS)
        if total > 0:
            grand_total += total
            category_totals["CMake"] += total
            print(format_output("CMake", script_name, total, script_dir))
    
    print()
    
    # Discover and count Shader code
    print("Shader Code:")
    print("-" * 60)
    
    shader_dirs = discover_shader_dirs()
    for shader_name in sorted(shader_dirs.keys()):
        shader_dir = shader_dirs[shader_name]
        total = count_lines_in_dir(shader_dir, SHADER_EXTS)
        if total > 0:
            grand_total += total
            category_totals["Shader"] += total
            print(format_output("Shader", shader_name, total, shader_dir))
    
    print()
    
    # Discover and count C# code
    print("C# Code:")
    print("-" * 60)
    
    csharp_dirs = discover_csharp_dirs()
    for csharp_name in sorted(csharp_dirs.keys()):
        csharp_dir = csharp_dirs[csharp_name]
        total = count_lines_in_dir(csharp_dir, CSHARP_EXTS)
        if total > 0:
            grand_total += total
            category_totals["C#"] += total
            print(format_output("C#", csharp_name, total, csharp_dir))
    
    print()
    print("=" * 60)
    print("Summary by Category:")
    for category in sorted(category_totals.keys()):
        print(f"  {category:<8}: {category_totals[category]:>10} LOC")
    print("=" * 60)
    print(f"TOTAL: {grand_total:>10} LOC")
    print()


if __name__ == "__main__":
    main()
