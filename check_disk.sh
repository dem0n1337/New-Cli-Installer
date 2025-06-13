#!/bin/bash

echo "=== Checking disk nvme3n1 status ==="
echo ""

# Check if disk exists
if [ ! -e /dev/nvme3n1 ]; then
    echo "ERROR: /dev/nvme3n1 does not exist!"
    exit 1
fi

echo "1. Basic disk information:"
lsblk /dev/nvme3n1

echo ""
echo "2. Checking for existing partitions:"
fdisk -l /dev/nvme3n1

echo ""
echo "3. Checking SMART status (if available):"
nvme smart-log /dev/nvme3n1 2>/dev/null || echo "SMART data not available"

echo ""
echo "4. Checking for disk errors in dmesg:"
dmesg | grep -i nvme3n1 | tail -20

echo ""
echo "5. Checking if disk is mounted:"
mount | grep nvme3n1

echo ""
echo "6. Checking disk size and model:"
nvme id-ctrl /dev/nvme3n1 2>/dev/null | grep -E "(mn|sn|fr)" || echo "Cannot get NVMe info"

echo ""
echo "7. Testing if disk is writable:"
dd if=/dev/zero of=/dev/nvme3n1 bs=512 count=1 2>&1 | grep -E "(error|failed|readonly)" && echo "Disk may have write issues" || echo "Disk appears writable"
