from pathlib import Path
import re
import os
import mkdocs_gen_files


PROJECT_ROOT = Path(__file__).resolve().parents[3]

# LICENSE
license_file = PROJECT_ROOT / "LICENSE"

with license_file.open(encoding="utf-8") as f:
    license_text = f.read()

with mkdocs_gen_files.open("LICENSE.md", "w") as fd:
    fd.write(license_text)


# README.md
readme_file = PROJECT_ROOT / "README.md"

with readme_file.open(encoding="utf-8") as f:
    readme = f.read()

with mkdocs_gen_files.open("README.md", "w") as fd:
    content = readme.replace('<img src="docs/', '<img src="').replace('href="docs/', 'href="').replace('[here](/api/index.html)','[here](/source/api)')
    fd.write(content)

# conclusions.md
conclusions_file = PROJECT_ROOT / "docs" / "conclusions.md"

with conclusions_file.open(encoding="utf-8") as f:
    conclusions = f.read()

with mkdocs_gen_files.open("conclusions.md", "w") as fd:
    content = conclusions.replace('<img src="docs/', '<img src="').replace('../LICENSE', 'LICENSE.md')
    fd.write(content)
