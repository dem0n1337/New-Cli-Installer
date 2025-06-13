#!/bin/bash

# Test script for CachyOS installer RAID functionality

echo "=== CachyOS Installer RAID Test ==="
echo "This script will create test loop devices to simulate disks for RAID testing"
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Clean up any existing test devices
echo "Cleaning up any existing test devices..."
for i in {0..3}; do
    if [ -e /dev/loop$i ]; then
        mdadm --stop /dev/md/test_raid0 2>/dev/null
        mdadm --stop /dev/md/test_raid1 2>/dev/null
        losetup -d /dev/loop$i 2>/dev/null
    fi
    rm -f /tmp/disk$i.img 2>/dev/null
done

# Create test disk images
echo "Creating test disk images..."
for i in {0..3}; do
    dd if=/dev/zero of=/tmp/disk$i.img bs=1M count=100 2>/dev/null
    losetup /dev/loop$i /tmp/disk$i.img
    echo "Created /dev/loop$i (100MB)"
done

echo ""
echo "Test devices created:"
losetup -a | grep "/tmp/disk"

echo ""
echo "You can now run the installer and test RAID creation with these devices:"
echo "  - /dev/loop0 and /dev/loop1 for RAID 0"
echo "  - /dev/loop2 and /dev/loop3 for RAID 1"
echo ""
echo "To clean up after testing, run: $0 cleanup"

if [ "$1" == "cleanup" ]; then
    echo "Cleaning up test devices..."
    mdadm --stop /dev/md/* 2>/dev/null
    for i in {0..3}; do
        losetup -d /dev/loop$i 2>/dev/null
        rm -f /tmp/disk$i.img
    done
    echo "Cleanup complete"
fi
