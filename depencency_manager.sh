#!/bin/sh

if [ "$(uname -s)" != "Linux" ]; then
    echo "This project requires a Linux kernel with eBPF/TC support."
    exit 1
fi

APT_DEPS="libbpf-dev \
          libelf-dev \
          libreadline-dev \
          iproute2"

YUM_DEPS="libbpf-devel \
          elfutils-libelf-devel \
          readline-devel \
          iproute"

APK_DEPS="libbpf-dev \
          elfutils-dev \
          readline-dev \
          iproute2"

install_deps()
{
    if command -v apt >/dev/null 2>&1; then
        sudo apt update
        sudo apt install -y $APT_DEPS make gcc clang

    elif command -v yum >/dev/null 2>&1; then
        sudo yum install -y $YUM_DEPS make gcc clang

    elif command -v apk >/dev/null 2>&1; then
        sudo apk add $APK_DEPS make gcc clang musl-dev

    else
        echo "Unsupported distribution"
        exit 1
    fi
}

uninstall_deps()
{
    if command -v apt >/dev/null 2>&1; then
        sudo apt remove -y --purge $APT_DEPS
        sudo apt autoremove -y

    elif command -v yum >/dev/null 2>&1; then
        sudo yum remove -y $YUM_DEPS
        sudo yum autoremove -y

    elif command -v apk >/dev/null 2>&1; then
        sudo apk del $APK_DEPS

    else
        echo "Unsupported distribution"
        exit 1
    fi
}

case "$1" in
    install)
        echo "Installing dependencies..."
        install_deps
        ;;
    uninstall)
        echo "Uninstalling dependencies..."
        uninstall_deps
        ;;
    *)
        echo "Usage: $0 {install|uninstall}"
        exit 1
        ;;
esac