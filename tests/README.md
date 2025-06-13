# RAID and LVM Compatibility Testing

This directory contains tests to verify the compatibility between RAID and LVM in the CLI installer.

## Background

The CLI installer has been updated to improve compatibility between RAID arrays and Logical Volume Management (LVM). The following issues were addressed:

1. Syntax errors in RAID creation commands
2. Missing RAID detection in configuration
3. Missing RAID modules in initramfs
4. Incomplete error handling for RAID+LVM operations
5. Lack of diagnostic logging

## Testing the Changes

### Automated Testing

The `raid_lvm_test.sh` script provides an automated test for RAID and LVM compatibility. It:

1. Creates virtual disk files
2. Sets up a RAID 1 array
3. Creates LVM structures on the RAID array
4. Creates and mounts a filesystem
5. Verifies data can be written and read
6. Checks initramfs configuration

To run the automated test:

```bash
sudo bash tests/raid_lvm_test.sh
```

The test results will be logged to `/tmp/raid_lvm_test.log`.

### Manual Testing

For a more thorough test, follow these steps in a virtual machine or test environment:

1. **Create RAID Array**:
   - Use the installer's RAID menu to create a RAID 0 or RAID 1 array
   - Verify the array is created successfully with `cat /proc/mdstat`

2. **Create LVM on RAID**:
   - Use the installer's LVM menu to create a volume group on the RAID array
   - Create logical volumes on this volume group
   - Verify with `pvs`, `vgs`, and `lvs` commands

3. **Install System**:
   - Install the system using the LVM logical volumes
   - Ensure you select the appropriate mount points

4. **Verify Boot Configuration**:
   - Check that `/etc/mkinitcpio.conf` includes the `mdadm_udev` hook
   - Verify that `/etc/mdadm.conf` contains the RAID configuration
   - Run `mkinitcpio -P` to rebuild the initramfs

5. **Reboot and Test**:
   - Reboot the system
   - Verify that the system boots correctly
   - Check that the RAID array and LVM volumes are properly mounted

## Diagnostic Information

When using both RAID and LVM, the installer now logs diagnostic information to `/tmp/cachyos-install.log`, including:

- RAID array status from `/proc/mdstat`
- RAID configuration from `mdadm --detail --scan`
- LVM physical volumes, volume groups, and logical volumes
- Initramfs configuration

Review this log file if you encounter any issues.

## Known Limitations

- The current implementation only supports RAID 0 (striping) and RAID 1 (mirroring)
- More complex RAID levels (RAID 5, RAID 6, RAID 10) are not supported
- Nested RAID configurations may not work correctly