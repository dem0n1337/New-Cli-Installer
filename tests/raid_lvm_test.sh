#!/bin/bash
# Test script for RAID and LVM compatibility
# This script should be run in a test environment with virtual disks

set -e
LOG_FILE="/tmp/raid_lvm_test.log"

echo "=== Starting RAID+LVM compatibility test ===" | tee -a $LOG_FILE
date | tee -a $LOG_FILE

# Function to log messages
log() {
    echo "[$(date +%T)] $1" | tee -a $LOG_FILE
}

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    log "This script must be run as root"
    exit 1
fi

# Create virtual disks for testing
log "Creating virtual disk files..."
dd if=/dev/zero of=/tmp/disk1.img bs=1M count=512 &>/dev/null
dd if=/dev/zero of=/tmp/disk2.img bs=1M count=512 &>/dev/null

# Set up loop devices
log "Setting up loop devices..."
LOOP1=$(losetup -f --show /tmp/disk1.img)
LOOP2=$(losetup -f --show /tmp/disk2.img)
log "Loop devices: $LOOP1 and $LOOP2"

# Clean up function
cleanup() {
    log "Cleaning up..."
    # Stop RAID array if it exists
    if [ -e /dev/md0 ]; then
        mdadm --stop /dev/md0 || true
    fi
    
    # Remove LVM structures if they exist
    if lvs | grep -q testvg; then
        lvremove -f /dev/testvg/testlv || true
    fi
    if vgs | grep -q testvg; then
        vgremove -f testvg || true
    fi
    if pvs | grep -q "$LOOP1\|$LOOP2\|/dev/md0"; then
        pvremove -f $LOOP1 $LOOP2 /dev/md0 || true
    fi
    
    # Detach loop devices
    losetup -d $LOOP1 || true
    losetup -d $LOOP2 || true
    
    # Remove disk images
    rm -f /tmp/disk1.img /tmp/disk2.img
    
    log "Cleanup complete"
}

# Register cleanup function to run on exit
trap cleanup EXIT

# Test 1: Create RAID array
log "Test 1: Creating RAID 1 array..."
mdadm --create --verbose /dev/md0 --level=1 --raid-devices=2 $LOOP1 $LOOP2 --run
mdadm --detail /dev/md0 | tee -a $LOG_FILE
log "RAID array created successfully"

# Test 2: Create LVM on RAID
log "Test 2: Creating LVM on RAID..."
pvcreate -ff /dev/md0
pvs | tee -a $LOG_FILE
vgcreate testvg /dev/md0
vgs | tee -a $LOG_FILE
lvcreate -L 100M -n testlv testvg
lvs | tee -a $LOG_FILE
log "LVM on RAID created successfully"

# Test 3: Create filesystem on LVM
log "Test 3: Creating filesystem on LVM volume..."
mkfs.ext4 /dev/testvg/testlv
mkdir -p /mnt/test
mount /dev/testvg/testlv /mnt/test
df -h /mnt/test | tee -a $LOG_FILE
log "Filesystem created and mounted successfully"

# Test 4: Write and read data
log "Test 4: Writing and reading data..."
echo "This is a test file for RAID+LVM compatibility" > /mnt/test/testfile.txt
cat /mnt/test/testfile.txt | tee -a $LOG_FILE
log "Data written and read successfully"

# Test 5: Simulate initramfs configuration
log "Test 5: Checking initramfs configuration..."
echo "HOOKS=(base udev autodetect modconf block mdadm_udev lvm2 filesystems keyboard fsck)" > /tmp/mkinitcpio.conf
cat /tmp/mkinitcpio.conf | tee -a $LOG_FILE
log "Initramfs configuration created successfully"

# Unmount filesystem
umount /mnt/test

log "All tests completed successfully!"
echo "=== RAID+LVM compatibility test completed ===" | tee -a $LOG_FILE