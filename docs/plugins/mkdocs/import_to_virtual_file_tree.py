from pathlib import Path
import re
import os
import mkdocs_gen_files

"""
Plugin que copia README.md e LICENSE para o virtual MkDocs file tree
e adapta links entre ficheiros Markdown para paths corretos no build.
"""

PROJECT_ROOT = Path(__file__).resolve().parents[3]

# -------------------------
# LICENSE
# -------------------------
license_file = PROJECT_ROOT / "LICENSE"

with license_file.open(encoding="utf-8") as f:
    license_text = f.read()

with mkdocs_gen_files.open("LICENSE", "w") as fd:
    fd.write(license_text)


# -------------------------
# README.md
# -------------------------
readme_file = PROJECT_ROOT / "README.md"

with readme_file.open(encoding="utf-8") as f:
    readme = f.read()

with mkdocs_gen_files.open("README.md", "w") as fd:
    fd.write(readme.replace('<img src="docs/', '<img src="'))

