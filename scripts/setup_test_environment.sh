#!/bin/sh
set -eu

NS1="nspace1"
NS2="nspace2"

BR="br0"

VETH1="veth1"
VETH1_BR="veth1-br"

VETH2="veth2"
VETH2_BR="veth2-br"

IP1="10.0.0.1/30"
IP2="10.0.0.2/30"

netns_exists() {
    ip netns list | grep -qw "$1"
}

link_exists() {
    ip link show "$1" >/dev/null 2>&1
}

cleanup_test_environment() {
    echo "[*] Cleaning up test environment..."

    if netns_exists "$NS1"; then
        sudo ip netns delete "$NS1" || true
    fi

    if netns_exists "$NS2"; then
        sudo ip netns delete "$NS2" || true
    fi

    link_exists "$VETH1_BR" && sudo ip link delete "$VETH1_BR" || true
    link_exists "$VETH2_BR" && sudo ip link delete "$VETH2_BR" || true

    link_exists "$BR" && sudo ip link delete "$BR" || true

    echo "[*] Test environment cleaned up."
}

setup_test_environment() {
    echo "[*] Setting up test environment..."

    # Garante estado limpo
    cleanup_test_environment

    echo "[*] Creating network namespaces..."
    sudo ip netns add "$NS1"
    sudo ip netns add "$NS2"

    echo "[*] Creating bridge..."
    sudo ip link add "$BR" type bridge
    sudo ip link set "$BR" up

    echo "[*] Creating veth pairs..."
    sudo ip link add "$VETH1" type veth peer name "$VETH1_BR"
    sudo ip link add "$VETH2" type veth peer name "$VETH2_BR"

    echo "[*] Moving interfaces to namespaces..."
    sudo ip link set "$VETH1" netns "$NS1"
    sudo ip link set "$VETH2" netns "$NS2"

    echo "[*] Connecting bridge ports..."
    sudo ip link set "$VETH1_BR" master "$BR"
    sudo ip link set "$VETH2_BR" master "$BR"

    sudo ip link set "$VETH1_BR" up
    sudo ip link set "$VETH2_BR" up

    echo "[*] Bringing up loopback..."
    sudo ip netns exec "$NS1" ip link set lo up
    sudo ip netns exec "$NS2" ip link set lo up

    echo "[*] Configuring IP addresses..."
    sudo ip netns exec "$NS1" ip addr replace "$IP1" dev "$VETH1"
    sudo ip netns exec "$NS2" ip addr replace "$IP2" dev "$VETH2"

    sudo ip netns exec "$NS1" ip link set "$VETH1" up
    sudo ip netns exec "$NS2" ip link set "$VETH2" up

    echo "[*] Environment ready."
}

case "${1:-}" in
    setup)
        setup_test_environment
        ;;
    cleanup)
        cleanup_test_environment
        ;;
    *)
        echo "Usage: $0 {setup|cleanup}"
        exit 1
        ;;
esac