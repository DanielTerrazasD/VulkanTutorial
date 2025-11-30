import os
import subprocess
import sys
from pathlib import Path
import shutil
import glob


def find_glslc():
    """
    Attempts to locate glslc.exe.
    Returns a full path to glslc.exe or None.
    Search order:
        1. PATH
        2. VULKAN_SDK environment variable
        3. Default VulkanSDK install locations on Windows
    """

    # -------------------------------------------------------
    # 1. Check PATH
    # -------------------------------------------------------
    glslc = shutil.which("glslc.exe")
    if glslc:
        return Path(glslc)

    # -------------------------------------------------------
    # 2. Check VULKAN_SDK environment variable
    # -------------------------------------------------------
    vulkan_sdk = os.environ.get("VULKAN_SDK")
    if vulkan_sdk:
        guess = Path(vulkan_sdk) / "Bin" / "glslc.exe"
        if guess.exists():
            return guess

    # -------------------------------------------------------
    # 3. Search common Vulkan SDK install dir
    # -------------------------------------------------------
    default_sdk_dir = Path("C:/VulkanSDK")
    if default_sdk_dir.exists():
        # Look for any version
        for p in default_sdk_dir.iterdir():
            guess = p / "Bin" / "glslc.exe"
            if guess.exists():
                return guess

    return None


# --------------------------------------------------------------------
# Main compile logic
# --------------------------------------------------------------------

SHADER_DIR = "../shaders"
OUT_DIR = "../shaders"


def compile_shader(glslc_path, input_path: Path, output_path: Path):
    cmd = [
        str(glslc_path),
        str(input_path),
        "-o", str(output_path),
    ]

    # print(" ".join(cmd))
    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"[FAILED] {input_path}")
        print(result.stderr)
    else:
        print(f"[OK] {input_path.name} -> {output_path.name}")


def main():
    glslc = find_glslc()
    if not glslc:
        print("ERROR: Could not locate glslc.exe on this system.")
        print("Make sure the Vulkan SDK is installed or glslc is in PATH.")
        sys.exit(1)

    print(f"Using glslc: {glslc}")

    shader_dir = Path(SHADER_DIR)
    output_dir = Path(OUT_DIR)
    output_dir.mkdir(exist_ok=True)

    shader_exts = {
        ".vert", ".frag", ".comp", ".geom", ".tesc", ".tese",
        ".mesh", ".task", ".rgen", ".rchit", ".rmiss"
    }

    for shader in shader_dir.glob("*"):
        if shader.suffix.lower() in shader_exts:
            out = output_dir / (shader.name + ".spv")
            compile_shader(glslc, shader, out)

    print("Done.")


if __name__ == "__main__":
    main()
