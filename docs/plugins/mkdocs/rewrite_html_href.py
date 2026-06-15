import re
from pathlib import PurePosixPath


def on_page_markdown(markdown, page, config, files, **kwargs):

    current_dir = PurePosixPath(page.file.src_path).parent

    # Diretório do URL de output — onde o browser "está"
    # ex: source/development/developing-new-modules/index.html → source/development/developing-new-modules
    current_url_dir = PurePosixPath(page.file.dest_uri).parent

    pattern = r"""(<a\s[^>]*href=)(["'])([^"']+)(["'])"""

    def resolve(href):
        if href.startswith(("http://", "https://", "#", "mailto:", "/")):
            return None

        fragment = ""
        if "#" in href:
            href, fragment = href.split("#", 1)
            fragment = "#" + fragment

        if not href:
            return None

        resolved_str = str(_normalize_path(current_dir / href))

        if resolved_str.endswith(".md"):
            resolved_str = resolved_str[:-3]

        target_file = files.get_file_from_path(resolved_str + ".md")
        if not target_file:
            for f in files:
                src_no_ext = f.src_uri[:-3] if f.src_uri.endswith(".md") else f.src_uri
                if src_no_ext.lstrip("/") == resolved_str.lstrip("/"):
                    target_file = f
                    break

        if not target_file:
            return None

        # Diretório do URL de output do destino
        dst_url_dir = PurePosixPath(target_file.dest_uri).parent

        relative = _relative_url(current_url_dir, dst_url_dir)
        return relative + fragment

    def replacer(match):
        prefix, q_open, href, q_close = match.groups()
        new_url = resolve(href)
        if new_url is None:
            return match.group(0)
        return f"{prefix}{q_open}{new_url}{q_close}"

    return re.sub(pattern, replacer, markdown, flags=re.DOTALL)


def _relative_url(src_dir: PurePosixPath, dst_dir: PurePosixPath) -> str:
    src_parts = list(src_dir.parts) if str(src_dir) != "." else []
    dst_parts = list(dst_dir.parts) if str(dst_dir) != "." else []

    common_len = 0
    for a, b in zip(src_parts, dst_parts):
        if a == b:
            common_len += 1
        else:
            break

    ups = len(src_parts) - common_len
    down = dst_parts[common_len:]

    rel_parts = [".."] * ups + down
    return "/".join(rel_parts) if rel_parts else "."


def _normalize_path(path) -> PurePosixPath:
    parts = []
    for part in PurePosixPath(path).parts:
        if part == "..":
            if parts and parts[-1] not in ("", ".."):
                parts.pop()
        elif part != ".":
            parts.append(part)
    return PurePosixPath(*parts) if parts else PurePosixPath(".")
