#!/bin/sh

set -eu

VERBOSE=${VERBOSE:-0}
[ "$VERBOSE" = "1" ] && set -x

[ "$(uname -s)" = "Linux" ] || {
    echo "[!] This project requires Linux."
    exit 1
}

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECT_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

INCLUDE_DIR="$PROJECT_ROOT/src/include"
VMLINUX_H="$INCLUDE_DIR/vmlinux.h"

SUDO=${SUDO:-sudo}
TMP_DIR=${TMPDIR:-/tmp}

LIBBPF_DIR="$TMP_DIR/libbpf"
LINUX_DIR="$TMP_DIR/linux-bpftool"

KERNEL_RELEASE=$(uname -r)

apt_install() {
    echo "[*] apt-get install: $*"
    $SUDO apt-get update
    $SUDO apt-get install -y "$@"
}

install_deps() {
    echo "[*] Installing dependencies..."

    if command -v apt-get >/dev/null 2>&1; then
        echo "[*] Detected apt-based system"

        apt_install \
            git clang llvm gcc make pkg-config \
            libelf-dev zlib1g-dev libreadline-dev iproute2

    elif command -v yum >/dev/null 2>&1; then
        echo "[*] Detected yum-based system"

        $SUDO yum install -y \
            git clang llvm gcc make pkgconf-pkg-config \
            elfutils-libelf-devel zlib-devel readline-devel iproute

    elif command -v apk >/dev/null 2>&1; then
        echo "[*] Detected Alpine system"

        $SUDO apk add \
            git clang llvm gcc make pkgconf \
            elfutils-dev zlib-dev readline-dev iproute2 musl-dev

    else
        echo "[!] Unsupported distribution."
        exit 1
    fi
}

install_libbpf() {
    PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:${PKG_CONFIG_PATH:-}"

    if PKG_CONFIG_PATH="$PKG_CONFIG_PATH" pkg-config --exists libbpf 2>/dev/null; then
        echo "[*] libbpf already installed."
        echo "[*] libbpf version: $(PKG_CONFIG_PATH="$PKG_CONFIG_PATH" pkg-config --modversion libbpf)"
        return
    fi

    echo "[*] Installing libbpf from source..."
    echo "[*] Source directory: $LIBBPF_DIR"

    rm -rf "$LIBBPF_DIR"

    git clone --depth=1 https://github.com/libbpf/libbpf.git "$LIBBPF_DIR"

    echo "[*] Building libbpf..."
    make -C "$LIBBPF_DIR/src"

    echo "[*] Installing libbpf..."
    $SUDO make -C "$LIBBPF_DIR/src" install

    if command -v ldconfig >/dev/null 2>&1; then
        echo "[*] Running ldconfig..."
        $SUDO ldconfig
    fi

    echo "[+] libbpf installed."
}

bpftool_works() {
    [ -n "${1:-}" ] || return 1
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

        if bpftool_works "$tool"; then
            echo "$tool"
            return 0
        fi
    done

    if command -v bpftool >/dev/null 2>&1; then
        tool=$(command -v bpftool)

        if bpftool_works "$tool"; then
            echo "$tool"
            return 0
        fi
    fi

    return 1
}

install_bpftool_from_packages() {
    command -v apt-get >/dev/null 2>&1 || return 1

    echo "[*] Trying bpftool packages for kernel: $KERNEL_RELEASE"

    for pkg in \
        "linux-tools-$KERNEL_RELEASE" \
        "linux-tools-$KERNEL_RELEASE-standard-WSL2" \
        "linux-cloud-tools-$KERNEL_RELEASE-standard-WSL2" \
        "linux-tools-standard-WSL2" \
        "linux-cloud-tools-standard-WSL2" \
        linux-tools-generic \
        linux-tools-common \
        bpftool
    do
        echo "[*] Checking package: $pkg"

        if apt-cache show "$pkg" >/dev/null 2>&1; then
            echo "[*] Installing package: $pkg"
            apt_install "$pkg"

            if BPFTool=$(find_bpftool); then
                echo "[+] Working bpftool found: $BPFTool"
                return 0
            fi
        fi
    done

    echo "[*] No working packaged bpftool found."
    return 1
}

install_bpftool() {
    if BPFTool=$(find_bpftool); then
        echo "[*] bpftool already working: $BPFTool"
        return
    fi

    echo "[*] No working bpftool found."

    if install_bpftool_from_packages; then
        return
    fi

    echo "[*] Building bpftool from Linux source..."
    echo "[*] Source directory: $LINUX_DIR"

    rm -rf "$LINUX_DIR"

    git clone --depth=1 https://github.com/torvalds/linux.git "$LINUX_DIR"

    make -C "$LINUX_DIR/tools/bpf/bpftool"
    $SUDO make -C "$LINUX_DIR/tools/bpf/bpftool" install

    if BPFTool=$(find_bpftool); then
        echo "[+] bpftool installed from source: $BPFTool"
        return
    fi

    echo "[!] bpftool was built, but it still does not work."
    exit 1
}

generate_vmlinux_h() {
    echo "[*] Generating vmlinux.h..."

    mkdir -p "$INCLUDE_DIR"

    [ -r /sys/kernel/btf/vmlinux ] || {
        echo "[!] /sys/kernel/btf/vmlinux not found."
        echo "[!] Your kernel may not have BTF support enabled."
        exit 1
    }

    BPFTool=$(find_bpftool || true)

    [ -n "$BPFTool" ] || {
        echo "[!] No working bpftool found."
        exit 1
    }

    echo "[*] Using bpftool: $BPFTool"

    "$BPFTool" btf dump file /sys/kernel/btf/vmlinux format c > "$VMLINUX_H"

    echo "[+] Generated: $VMLINUX_H"
}

uninstall() {
    echo "[*] Removing non-critical files installed by this script..."

    rm -f "$VMLINUX_H"
    rm -rf "$LIBBPF_DIR" "$LINUX_DIR"

    $SUDO rm -f /usr/local/lib/libbpf.*
    $SUDO rm -f /usr/local/lib64/libbpf.*
    $SUDO rm -f /usr/local/lib/pkgconfig/libbpf.pc
    $SUDO rm -f /usr/local/lib64/pkgconfig/libbpf.pc
    $SUDO rm -rf /usr/local/include/bpf

    $SUDO rm -f /usr/local/sbin/bpftool
    $SUDO rm -f /usr/local/bin/bpftool
    $SUDO rm -f /usr/local/share/bash-completion/completions/bpftool

    if command -v ldconfig >/dev/null 2>&1; then
        echo "[*] Running ldconfig..."
        $SUDO ldconfig
    fi

    echo "[+] Uninstall complete."
}

case "${1:-}" in
    install)
        install_deps
        install_libbpf
        install_bpftool
        generate_vmlinux_h

        echo
        echo "[+] Installation complete."
        ;;
    uninstall)
        uninstall
        ;;
    *)
        echo "Usage: $0 {install|uninstall}"
        echo "Debug: VERBOSE=1 $0 install"
        exit 1
        ;;
esac
