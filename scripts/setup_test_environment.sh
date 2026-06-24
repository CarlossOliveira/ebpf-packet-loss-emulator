#!/bin/sh
set -eu

NS1="nspace1"
NS2="nspace2"

BR="br0"

VETH1="veth1"
VETH1_BR="veth1-br"

VETH2="veth2"
VETH2_BR="veth2-br"

IP1="10.0.0.1/24"
IP2="10.0.0.2/24"

cleanup() {
    echo "[*] Cleanup..."

    ip netns del "$NS1" 2>/dev/null || true
    ip netns del "$NS2" 2>/dev/null || true
    ip link del "$BR" 2>/dev/null || true

    echo "[*] Done."
}

setup() {
    echo "[*] Setting up clean bridge datapath..."

    cleanup

    # namespaces
    ip netns add "$NS1"
    ip netns add "$NS2"

    # bridge
    ip link add "$BR" type bridge
    ip link set "$BR" up

    # disable offloads (evita bypass estranho)
    ethtool -K "$BR" gro off gso off tso off 2>/dev/null || true

    # veth pairs
    ip link add "$VETH1" type veth peer name "$VETH1_BR"
    ip link add "$VETH2" type veth peer name "$VETH2_BR"

    # move one side into namespaces
    ip link set "$VETH1" netns "$NS1"
    ip link set "$VETH2" netns "$NS2"

    # attach bridge side
    ip link set "$VETH1_BR" master "$BR"
    ip link set "$VETH2_BR" master "$BR"

    ip link set "$VETH1_BR" up
    ip link set "$VETH2_BR" up

    # bring up namespace interfaces
    ip netns exec "$NS1" ip link set lo up
    ip netns exec "$NS2" ip link set lo up

    ip netns exec "$NS1" ip link set "$VETH1" up
    ip netns exec "$NS2" ip link set "$VETH2" up

    # IPs directly on veth (CORRETO)
    ip netns exec "$NS1" ip addr add "$IP1" dev "$VETH1"
    ip netns exec "$NS2" ip addr add "$IP2" dev "$VETH2"

    echo "[*] Forcing ARP resolution..."
    ip netns exec "$NS1" ping -c1 10.0.0.2 >/dev/null || true
    ip netns exec "$NS2" ping -c1 10.0.0.1 >/dev/null || true

    echo "[*] DONE. Traffic goes through br0."
}

case "${1:-}" in
    setup)
        setup
        ;;
    cleanup)
        cleanup
        ;;
    *)
        echo "Usage: $0 {setup|cleanup}"
        exit 1
        ;;
esac