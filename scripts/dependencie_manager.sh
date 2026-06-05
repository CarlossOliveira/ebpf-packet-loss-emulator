#!/bin/sh

set -e

if [ "$(uname -s)" != "Linux" ]; then
    echo "This project requires Linux."
    exit 1
fi

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECT_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

INCLUDE_DIR="$PROJECT_ROOT/src/include"
VMLINUX_H="$INCLUDE_DIR/vmlinux.h"

LIBBPF_DIR="/tmp/libbpf"
BPFTool_DIR="/tmp/bpftool"

APT_DEPS="git clang llvm gcc make pkg-config libelf-dev zlib1g-dev libreadline-dev iproute2"
YUM_DEPS="git clang llvm gcc make pkgconf-pkg-config elfutils-libelf-devel zlib-devel readline-devel iproute"
APK_DEPS="git clang llvm gcc make pkgconf elfutils-dev zlib-dev readline-dev iproute2 musl-dev"

install_deps()
{
    if command -v apt >/dev/null 2>&1; then
        sudo apt update
        sudo apt install -y $APT_DEPS
    elif command -v yum >/dev/null 2>&1; then
        sudo yum install -y $YUM_DEPS
    elif command -v apk >/dev/null 2>&1; then
        sudo apk add $APK_DEPS
    else
        echo "Unsupported distribution"
        exit 1
    fi
}

install_libbpf()
{
    rm -rf "$LIBBPF_DIR"
    git clone --depth=1 https://github.com/libbpf/libbpf.git "$LIBBPF_DIR"

    make -C "$LIBBPF_DIR/src"
    sudo make -C "$LIBBPF_DIR/src" install
    sudo ldconfig
}

install_bpftool()
{
    if command -v bpftool >/dev/null 2>&1; then
        return 0
    fi

    rm -rf "$BPFTool_DIR"
    git clone --depth=1 https://github.com/torvalds/linux.git "$BPFTool_DIR"

    make -C "$BPFTool_DIR/tools/bpf/bpftool"
    sudo make -C "$BPFTool_DIR/tools/bpf/bpftool" install
}

generate_vmlinux_h()
{
    mkdir -p "$INCLUDE_DIR"

    if [ ! -r /sys/kernel/btf/vmlinux ]; then
        echo "/sys/kernel/btf/vmlinux not found."
        exit 1
    fi

    bpftool btf dump file /sys/kernel/btf/vmlinux format c > "$VMLINUX_H"
    echo "Generated $VMLINUX_H"
}

uninstall()
{
    sudo rm -f /usr/local/lib/libbpf.*
    sudo rm -rf /usr/local/include/bpf
    sudo rm -f /usr/local/sbin/bpftool /usr/local/bin/bpftool
    sudo ldconfig

    rm -f "$VMLINUX_H"
    rm -rf "$LIBBPF_DIR" "$BPFTool_DIR"

    echo "Uninstalled generated vmlinux.h, local libbpf and local bpftool."
}

case "$1" in
    install)
        install_deps
        install_libbpf
        install_bpftool
        generate_vmlinux_h
        echo "Install complete."
        ;;
    uninstall)
        uninstall
        ;;
    *)
        echo "Usage: $0 {install|uninstall}"
        exit 1
        ;;
esac