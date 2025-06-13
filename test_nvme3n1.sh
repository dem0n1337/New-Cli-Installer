#!/bin/bash

echo "=== Testing nvme3n1 disk status ==="

if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Check for disk errors
echo "1. Checking for disk errors in kernel log:"
dmesg | grep -i "nvme3n1" | grep -E "(error|fail|timeout|reset)" | tail -10

echo ""
echo "2. Checking disk health:"
nvme smart-log /dev/nvme3n1 2>/dev/null | grep -E "(critical_warning|temperature|available_spare|percentage_used)" || echo "Cannot read SMART data"

echo ""
echo "3. Checking if disk is locked or in use:"
lsof /dev/nvme3n1* 2>/dev/null || echo "No processes using the disk"

echo ""
echo "4. Testing simple write operation:"
dd if=/dev/zero of=/dev/nvme3n1 bs=512 count=1 oflag=direct 2>&1 | grep -v "records"

echo ""
echo "5. Current partition state:"
parted /dev/nvme3n1 print 2>&1 | head -15

echo ""
echo "If you see I/O errors above, the disk may be failing."
echo "Try these commands to reset the disk:"
echo "  1. wipefs -a -f /dev/nvme3n1"
echo "  2. dd if=/dev/zero of=/dev/nvme3n1 bs=1M count=10"
echo "  3. partprobe /dev/nvme3n1"
