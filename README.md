# 🚀 cachyos-new-installer 🚀

![Demo image](./demo.gif)

CLI net-installer for CachyOS, inspired by manjaro-architect.

This installer provides online installation for CachyOS. It aims to be a fast and smooth installer for Arch-based operating systems. 💨

## ✨ Core Features

* 🌐 Online installation for CachyOS.
* 💾 Support for advanced storage setups including RAID and LVM.
* 🔒 LUKS encryption.
* 🛠️ Flexible partitioning and system configuration.

## 🎯 Current Development Focus

* **✅ Accomplished:**
  * Basic RAID 0/1 array creation.
  * Full LVM (PV/VG/LV) creation functionality - **not** fully working at the moment but it will be soon 😊
* **🚧 Work in Progress / To Do:**
  * Refining RAID management (deletion, diverse level support).
  * Comprehensive LUKS testing and integration.
  * Improving device listing and partitioning toolset.
  * Simpler layout and flow for the installer.
  * Future: Offline installation capabilities 📦.

For detailed progress, ongoing tasks, and specific technical updates, please see [CHANGELOG.md](./CHANGELOG.md) 📝.

## 📋 Requirements

* C++23 compatible compiler (tested with GCC 14.1.1 and Clang 18).

## 🛠️ Installing from source

This is tested on Arch Linux, but *any* recent Linux with a C++23 compiler should work:

```sh
sudo pacman -Sy \
    base-devel cmake pkg-config ninja clang mold llvm
```

### Cloning the source code ⤵️

```sh
git clone https://github.com/cachyos/new-cli-installer.git
cd new-cli-installer
```

### Building and Configuring ⚙️

#### CMake (recommended)

To build, first, configure it (if you intend to install it globally, you might also want `-DCMAKE_INSTALL_PREFIX=/usr`):

```sh
cmake -S . -B build
```

Second, build it:

```sh
cmake --build build
```

#### Meson

To build, first, configure it (if you intend to install it globally, you might also want `--prefix=/usr`):

```sh
meson setup build # meson build for older meson versions
```

Second, compile it:

```sh
meson compile -C build
```

Optionally, to disable developer environment:
Pass `-DENABLE_DEVENV=OFF` to cmake or `-Ddevenv=false` to meson when configuring the project.

## 📚 Libraries used in this project

* [Functional Terminal (X) User interface (FTXUI)](https://github.com/ArthurSonzogni/FTXUI)
* [fmt (A modern formatting library)](https://github.com/fmtlib/fmt)
* [spdlog (Fast C++ logging library)](https://github.com/gabime/spdlog)
* [simdjson (Parsing gigabytes of JSON per second)](https://github.com/simdjson/simdjson)
* [cpr (Curl for People)](https://github.com/libcpr/cpr)
* [range-v3](https://github.com/ericniebler/range-v3)

## 🗺️ Advanced menu overview

```text
Main Menu
|
├── Prepare Installation
|   ├── Set Virtual Console
|   ├── List Devices
|   ├── Partition Disk
|   ├── RAID (WIP - RAID 0/1 Creation OK)
|   ├── LUKS Encryption
|   ├── Logical Volume Management (Creation OK)
|   ├── Mount Partitions
|   ├── Configure Installer Mirrorlist
|   |   ├── Edit Pacman Configuration
|   |   └── Rank Mirrors by Speed (TODO)
|   |
│   └── Refresh Pacman Keys
|
├── Install System
│   ├── Install Base Packages
│   ├── Install Desktop
│   ├── Install Bootloader
│   ├── Configure Base
|   │   ├── Generate FSTAB
|   │   ├── Set Hostname
|   │   ├── Set System Locale
|   │   ├── Set Timezone and Clock
|   │   ├── Set Root Password
|   │   └── Add New User(s)
|   │
│   ├── Install Custom Packages
│   ├── System Tweaks
|   │   ├── Enable Automatic Login
|   │   └── Performance
|   |       ├── I/O schedulers
|   |       └── Swap configuration
|   │
│   ├── Review Configuration Files
│   └── Chroot into Installation
|
└── System Rescue
    ├── Install Hardware Drivers
    │   ├── Install Display Drivers
    │   └── Install Network Drivers (TODO)
    |
    ├── Install Bootloader
    ├── Configure Base
    |   └── ... (see 'Install System')
    │
    ├── Install Custom Packages
    ├── Remove Packages
    ├── Review Configuration Files
    ├── Chroot into Installation
    ├── Data Recovery (TODO)
    │   └── Btrfs snapshots..
    │
    └── View System Logs
        ├── Dmesg
        ├── Pacman log
        ├── Xorg log
        └── Journalctl
```
