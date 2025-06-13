# Changelog & Development Status

This document tracks development progress, known issues, and planned features for the CachyOS CLI Installer.

## LVM Implementation

* LVM menu added; submenu may need restructuring.
* **Status & Fixes:**
  * Initial LVM creation (LV, VG, PV) was not working; significant fixes implemented:
    * Added `get_kernel_params()` for bootloader parameters (LUKS, LVM, RAID).
    * Corrected `Initcpio` calls (e.g., `append_module`).
  * `mdadm_udev` hook included in `/etc/mkinitcpio.conf` for LVM on RAID.
* **Further Testing & Considerations:**
  * Comprehensive testing of various LVM configurations.
  * Verification of LV/VG/PV deletion.
  * Robustness of filesystem creation on LVs.
  * Reliable booting from LVM setups.
  * Consistent use of RAID arrays as LVM PVs.

## RAID Implementation

* RAID 0 and RAID 1 array creation tested successfully.
  * Syntax errors in creation commands fixed.
  * RAID detection added to installer configuration.
* **Needs further work on:**
  * RAID deletion functionality.
  * RAID data table creation (GPT/MBR).
  * Filesystem formatting on RAID arrays (or guidance for LVM use).
  * Potential support for other RAID levels (e.g., 5, 6, 10).
  * RAID array naming and consistent identification.
* **Key Fixes & Enhancements:**
  * `mdadm_udev` module added to initramfs.
  * Improved error handling for combined RAID+LVM operations.
  * Diagnostic logging for RAID operations incorporated.
  * System expected to boot correctly with RAID and LVM.
* Automated test script `tests/raid_lvm_test.sh` available for basic RAID/LVM interaction testing.

## Additional Notes & System Components

* **LUKS Functionality:**
  * Not extensively tested; expected to work (existed pre-fork). `get_kernel_params()` supports LUKS.
* **Device Listing:**
  * Current device listing is incomplete; needs rework for accurate display of all partitions (consolidating `lsblk`, `fdisk`, etc.).
* **Partitioning Submenu:**
  * Secure device wipe progress tracking broken.
  * **User Plan:** Investigate broken secure wipe. Streamline partitioning tools: remove `cgdisk`, `fdisk`, `gdisk`, `parted`; retain `cfdisk`. Check `secure data wipe` functionality and TUI progress tracking.
* **Bootloader & Kernel Configuration:**
  * `RefindInstallConfig`: fixed missing `extra_kernel_versions` field.
  * `get_kernel_params()` is crucial for correct kernel parameters.
* **Network & General Installer Features:**
  * `show_iwctl()` function added for `iwctl` wireless help.
  * Online installer (uses `pacstrap`). **User Plan:** Implement offline installation support in the future.
