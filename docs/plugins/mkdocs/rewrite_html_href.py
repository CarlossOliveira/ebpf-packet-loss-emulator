import re
from pathlib import PurePosixPath, Path
from mkdocs.structure.files import File


# Registry: populated in on_page_markdown, consumed in on_files (next build)
# For mkdocs serve we use on_page_content to inject into the live files object.
_pending_svgs: dict[str, Path] = {}  # dest_uri -> abs src path


def on_page_markdown(markdown, page, config, files, **kwargs):

    current_dir = PurePosixPath(page.file.src_path).parent
    current_url_dir = PurePosixPath(page.file.dest_uri).parent

    href_pattern = r"""(<a\s[^>]*href=)(["'])([^"']+)(["'])"""
    img_pattern  = r"""(<img\s[^>]*src=)(["'])([^"']+)(["'])"""

    # ------------------------------------------------------------------ #
    # href resolver                                                        #
    # ------------------------------------------------------------------ #
    def resolve_href(href):
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

        dst_url_dir = PurePosixPath(target_file.dest_uri).parent
        relative = _relative_url(current_url_dir, dst_url_dir)
        return relative + fragment

    # ------------------------------------------------------------------ #
    # img src resolver                                                     #
    #                                                                      #
    # Resolves the SVG on disk, registers it in _pending_svgs under the   #
    # dest_uri it should have (same URL dir as the page), and rewrites     #
    # the src to just the filename so it resolves correctly from the page. #
    # ------------------------------------------------------------------ #
    def resolve_img(src):
        if src.startswith(("http://", "https://", "/", "data:")):
            return None
        if not src.lower().endswith(".svg"):
            return None

        src_abs = (Path(page.file.abs_src_path).parent / src).resolve()
        if not src_abs.exists():
            return None

        # dest_uri = same folder as page output + just the filename
        dest_uri = str(current_url_dir / src_abs.name)

        # Add to virtual tree if not already there
        if not files.get_file_from_path(dest_uri):
            f = File(
                path=dest_uri,
                src_dir=None,          # generated file — no source dir
                dest_dir=config["site_dir"],
                use_directory_urls=config["use_directory_urls"],
                dest_uri=dest_uri,
            )
            f.content_bytes = src_abs.read_bytes()
            files.append(f)

        return src_abs.name

    def href_replacer(match):
        prefix, q_open, href, q_close = match.groups()
        new_url = resolve_href(href)
        if new_url is None:
            return match.group(0)
        return f"{prefix}{q_open}{new_url}{q_close}"

    def img_replacer(match):
        prefix, q_open, src, q_close = match.groups()
        new_src = resolve_img(src)
        if new_src is None:
            return match.group(0)
        return f"{prefix}{q_open}{new_src}{q_close}"

    markdown = re.sub(href_pattern, href_replacer, markdown, flags=re.DOTALL)
    markdown = re.sub(img_pattern,  img_replacer,  markdown, flags=re.DOTALL)

    return markdown


# ------------------------------------------------------------------ #
# Helpers                                                              #
# ------------------------------------------------------------------ #

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
