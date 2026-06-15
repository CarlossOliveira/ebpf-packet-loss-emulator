#!/bin/sh

set -eu

VERBOSE=${VERBOSE:-0}
[ "$VERBOSE" = "1" ] && set -x

# ------------------------------------------------------------
# System check
# ------------------------------------------------------------
[ "$(uname -s)" = "Linux" ] || {
    echo "[!] This project requires Linux."
    exit 1
}

# ------------------------------------------------------------
# Paths (ROOT is deterministic)
# ------------------------------------------------------------
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
PROJECT_ROOT=$(cd -- "$SCRIPT_DIR/.." && pwd -P)

INCLUDE_DIR="$PROJECT_ROOT/src/kernel-space/include"
VMLINUX_H="$INCLUDE_DIR/vmlinux.h"

TMP_DIR="${TMPDIR:-/tmp}"
LIBBPF_DIR="$TMP_DIR/libbpf"
LINUX_DIR="$TMP_DIR/linux-bpftool"

DOC_VENV="$PROJECT_ROOT/.docs-venv"

KERNEL_RELEASE=$(uname -r)

SUDO=${SUDO:-sudo}

# ------------------------------------------------------------
# Helpers
# ------------------------------------------------------------
apt_install() {
    echo "[*] apt-get install: $*"
    $SUDO apt-get update
    $SUDO apt-get install -y "$@"
}

# ------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------
install_deps() {
    echo "[*] Installing build dependencies..."

    if command -v apt-get >/dev/null 2>&1; then
        echo "[*] Detected apt-based system"

        apt_install \
            git clang llvm gcc make pkg-config \
            libelf-dev zlib1g-dev libreadline-dev iproute2 clang-format

    elif command -v yum >/dev/null 2>&1; then
        echo "[*] Detected yum-based system"

        $SUDO yum install -y \
            git clang llvm gcc make pkgconf-pkg-config \
            elfutils-libelf-devel zlib-devel readline-devel iproute clang-format

    elif command -v apk >/dev/null 2>&1; then
        echo "[*] Detected Alpine system"

        $SUDO apk add \
            git clang llvm gcc make pkgconf \
            elfutils-dev zlib-dev readline-dev iproute2 musl-dev clang-format

    else
        echo "[!] Unsupported distribution."
        exit 1
    fi
}

# ------------------------------------------------------------
# libbpf
# ------------------------------------------------------------
install_libbpf_from_packages() {
    if command -v apt-get >/dev/null 2>&1; then
        apt_install libbpf-dev
        track_pkg "libbpf-dev"
        return 0
    elif command -v yum >/dev/null 2>&1; then
        $SUDO yum install -y libbpf-devel
        return 0
    elif command -v apk >/dev/null 2>&1; then
        $SUDO apk add libbpf-dev
        return 0
    fi

    return 1
}

install_libbpf() {
    echo "[*] Installing libbpf..."

    PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:${PKG_CONFIG_PATH:-}"

    if PKG_CONFIG_PATH="$PKG_CONFIG_PATH" pkg-config --exists libbpf 2>/dev/null; then
        echo "[*] libbpf already available"
        return
    fi

    if install_libbpf_from_packages; then
        echo "[+] libbpf installed via package manager"
        return
    fi

    echo "[*] Building libbpf from source..."

    rm -rf "$LIBBPF_DIR"
    git clone --depth=1 https://github.com/libbpf/libbpf.git "$LIBBPF_DIR"

    make -C "$LIBBPF_DIR/src"
    $SUDO make -C "$LIBBPF_DIR/src" install

    if command -v ldconfig >/dev/null 2>&1; then
        $SUDO ldconfig
    fi
}

# ------------------------------------------------------------
# bpftool
# ------------------------------------------------------------
bpftool_works() {
    [ -x "$1" ] || return 1
    [ -r /sys/kernel/btf/vmlinux ] || return 1

    "$1" version >/dev/null 2>&1 || return 1
    "$1" btf dump file /sys/kernel/btf/vmlinux format c >/dev/null 2>&1 || return 1
}

find_bpftool() {
    for tool in \
        /usr/local/sbin/bpftool \
        /usr/local/bin/bpftool \
        "/usr/lib/linux-tools/$KERNEL_RELEASE/bpftool" \
        /usr/lib/linux-tools/*/bpftool
    do
        [ -x "$tool" ] || continue
        bpftool_works "$tool" && { echo "$tool"; return 0; }
    done

    if command -v bpftool >/dev/null 2>&1; then
        tool=$(command -v bpftool)
        bpftool_works "$tool" && { echo "$tool"; return 0; }
    fi

    return 1
}

install_bpftool() {
    echo "[*] Installing bpftool..."

    if find_bpftool >/dev/null 2>&1; then
        echo "[*] bpftool already working"
        return
    fi

    echo "[*] No packaged bpftool found, building from kernel..."

    rm -rf "$LINUX_DIR"
    git clone --depth=1 https://github.com/torvalds/linux.git "$LINUX_DIR"

    make -C "$LINUX_DIR/tools/bpf/bpftool"
    $SUDO make -C "$LINUX_DIR/tools/bpf/bpftool" install
}

# ------------------------------------------------------------
# vmlinux.h
# ------------------------------------------------------------
generate_vmlinux_h() {
    echo "[*] Generating vmlinux.h..."

    mkdir -p "$INCLUDE_DIR"

    [ -r /sys/kernel/btf/vmlinux ] || {
        echo "[!] BTF not available"
        exit 1
    }

    BPFTool=$(find_bpftool || true)

    [ -n "$BPFTool" ] || {
        echo "[!] bpftool not found"
        exit 1
    }

    "$BPFTool" btf dump file /sys/kernel/btf/vmlinux format c > "$VMLINUX_H"

    echo "[+] Generated $VMLINUX_H"
}

install_documentation_dependencies() {
    echo "[*] Installing documentation dependencies..."

    if command -v apt-get >/dev/null 2>&1; then
        apt_install python3-pip python3-venv doxygen graphviz
    elif command -v yum >/dev/null 2>&1; then
        $SUDO yum install -y python3-pip python3-venv doxygen graphviz
    elif command -v apk >/dev/null 2>&1; then
        $SUDO apk add python3 py3-pip py3-venv doxygen graphviz
    else
        echo "[!] Unsupported distribution."
        exit 1
    fi

    # --------------------------------------------------
    # Create venv only if it does not exist
    # --------------------------------------------------
    if [ ! -d "$DOC_VENV" ]; then
        echo "[*] Creating virtual environment..."
        python3 -m venv "$DOC_VENV"
    else
        echo "[*] Reusing existing virtual environment..."
    fi

    VENV_PY="$DOC_VENV/bin/python"

    # sanity check (prevents broken venv issues)
    if [ ! -x "$VENV_PY" ]; then
        echo "[!] Broken venv detected. Recreating..."
        rm -rf "$DOC_VENV"
        python3 -m venv "$DOC_VENV"
    fi

    # upgrade pip always (cheap + safe)
    "$VENV_PY" -m pip install --upgrade pip

    # install requirements only if needed
    if [ ! -f "$DOC_VENV/.deps_installed" ]; then
        echo "[*] Installing python dependencies..."
        "$VENV_PY" -m pip install -r "$PROJECT_ROOT/docs/requirements.txt"
        touch "$DOC_VENV/.deps_installed"
    else
        echo "[*] Python dependencies already installed"
    fi

    echo "[+] Docs environment ready."
}

# ------------------------------------------------------------
# Docs build
# ------------------------------------------------------------
build_docs() {
    echo "[*] Building docs..."

    mkdir -p "$PROJECT_ROOT/docs/source/api"

    doxygen "$(realpath "$PROJECT_ROOT/docs/Doxyfile")"

    MKDOCS="$DOC_VENV/bin/mkdocs"

    [ -x "$MKDOCS" ] || {
        echo "[!] mkdocs not installed in venv"
        exit 1
    }

    "$MKDOCS" build -f "docs/mkdocs.yml" -d "$PROJECT_ROOT/build/docs"

    echo "[+] Docs built"
}

uninstall() {
    echo "[*] Uninstalling project artifacts..."

    rm -f "$VMLINUX_H"
    rm -rf "$LIBBPF_DIR" "$LINUX_DIR"
    rm -rf "$DOC_VENV"

    $SUDO rm -f /usr/local/lib/libbpf.*
    $SUDO rm -f /usr/local/lib64/libbpf.*
    $SUDO rm -rf /usr/local/include/bpf
    $SUDO rm -f /usr/local/lib/pkgconfig/libbpf.pc

    $SUDO rm -f /usr/local/bin/bpftool
    $SUDO rm -f /usr/local/sbin/bpftool

    if command -v ldconfig >/dev/null 2>&1; then
        $SUDO ldconfig
    fi

    echo "[+] Uninstall complete"
}

# ------------------------------------------------------------
# CLI
# ------------------------------------------------------------
case "${1:-}" in
    install)
        install_deps
        install_libbpf
        install_bpftool
        generate_vmlinux_h
        echo "[+] Installation complete"
        ;;

    docs)
        install_documentation_dependencies
        build_docs
        ;;

    uninstall)
        uninstall
        ;;

    *)
        echo "Usage: $0 {install|docs|uninstall}"
        exit 1
        ;;
esac
