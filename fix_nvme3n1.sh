#!/bin/bash

echo "=== Attempting to fix nvme3n1 issues ==="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

echo "1. Checking current partition table:"
fdisk -l /dev/nvme3n1

echo ""
echo "2. Wiping existing partition signatures:"
wipefs -a /dev/nvme3n1

echo ""
echo "3. Creating fresh partition table:"
parted -s /dev/nvme3n1 mklabel gpt

echo ""
echo "4. Creating test partition:"
parted -s /dev/nvme3n1 mkpart primary ext4 1MiB 100%

echo ""
echo "5. Testing format with verbose output:"
mkfs.ext4 -v /dev/nvme3n1p1

echo ""
echo "6. Checking result:"
lsblk /dev/nvme3n1

echo ""
echo "If the format succeeded, you can now use the installer again."
echo "If it failed, check dmesg for hardware errors:"
echo "  dmesg | grep -i nvme3n1 | tail -20"
