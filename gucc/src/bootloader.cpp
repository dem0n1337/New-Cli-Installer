#include "gucc/bootloader.hpp"
#include "gucc/file_utils.hpp"
#include "gucc/initcpio.hpp"
#include "gucc/io_utils.hpp"
#include "gucc/kernel_params.hpp"
#include "gucc/string_utils.hpp"

#include <ranges>  // for ranges::*

#include <fmt/compile.h>
#include <fmt/format.h>

#include <spdlog/spdlog.h>
#include <string>

using namespace std::string_view_literals;

#define CONV_REQ_F(needle, f)               \
    if (line.starts_with(needle)) {         \
        return fmt::format(needle "{}", f); \
    }
#define CONV_REQ_F_S(needle, f)                 \
    if (line.starts_with(needle)) {             \
        return fmt::format(needle "\"{}\"", f); \
    }
#define CONV_OPT_F(needle, f, default_val)         \
    if (line.starts_with(needle)) {                \
        if (f) {                                   \
            return fmt::format(needle "{}", *(f)); \
        }                                          \
        return {"#" needle default_val};           \
    }
#define CONV_OPT_F_S(needle, f, default_val)           \
    if (line.starts_with(needle)) {                    \
        if (f) {                                       \
            return fmt::format(needle "\"{}\"", *(f)); \
        }                                              \
        return {"#" needle "\"" default_val "\""};     \
    }
#define CONV_REQ_B(needle, f)                                            \
    if (line.starts_with(needle)) {                                      \
        return fmt::format(needle "{}", convert_boolean_val(needle, f)); \
    }
#define CONV_OPT_B(needle, f, default_val)                                      \
    if (line.starts_with(needle)) {                                             \
        if (f) {                                                                \
            return fmt::format(needle "{}", convert_boolean_val(needle, *(f))); \
        }                                                                       \
        return {"#" needle default_val};                                        \
    }

namespace {

// NOLINTNEXTLINE
static constexpr auto GRUB_DEFAULT_CONFIG = R"(# GRUB boot loader configuration

GRUB_DEFAULT=0
GRUB_TIMEOUT=5
GRUB_DISTRIBUTOR="Arch"
GRUB_CMDLINE_LINUX_DEFAULT="loglevel=3 quiet"
GRUB_CMDLINE_LINUX=""

# Preload both GPT and MBR modules so that they are not missed
GRUB_PRELOAD_MODULES="part_gpt part_msdos"

# Uncomment to enable booting from LUKS encrypted devices
#GRUB_ENABLE_CRYPTODISK=y

# Set to 'countdown' or 'hidden' to change timeout behavior,
# press ESC key to display menu.
GRUB_TIMEOUT_STYLE=menu

# Uncomment to use basic console
GRUB_TERMINAL_INPUT=console

# Uncomment to disable graphical terminal
#GRUB_TERMINAL_OUTPUT=console

# The resolution used on graphical terminal
# note that you can use only modes which your graphic card supports via VBE
# you can see them in real GRUB with the command `videoinfo'
GRUB_GFXMODE=auto

# Uncomment to allow the kernel use the same resolution used by grub
GRUB_GFXPAYLOAD_LINUX=keep

# Uncomment if you want GRUB to pass to the Linux kernel the old parameter
# format "root=/dev/xxx" instead of "root=/dev/disk/by-uuid/xxx"
#GRUB_DISABLE_LINUX_UUID=true

# Uncomment to disable generation of recovery mode menu entries
GRUB_DISABLE_RECOVERY=true

# Uncomment and set to the desired menu colors.  Used by normal and wallpaper
# modes only.  Entries specified as foreground/background.
#GRUB_COLOR_NORMAL="light-blue/black"
#GRUB_COLOR_HIGHLIGHT="light-cyan/blue"

# Uncomment one of them for the gfx desired, a image background or a gfxtheme
#GRUB_BACKGROUND="/path/to/wallpaper"
#GRUB_THEME="/path/to/gfxtheme"

# Uncomment to get a beep at GRUB start
#GRUB_INIT_TUNE="480 440 1"

# Uncomment to make GRUB remember the last selection. This requires
# setting 'GRUB_DEFAULT=saved' above.
#GRUB_SAVEDEFAULT=true

# Uncomment to disable submenus in boot menu
#GRUB_DISABLE_SUBMENU=y

# Probing for other operating systems is disabled for security reasons. Read
# documentation on GRUB_DISABLE_OS_PROBER, if still want to enable this
# functionality install os-prober and uncomment to detect and include other
# operating systems.
#GRUB_DISABLE_OS_PROBER=false
)"sv;
// NOLINTNEXTLINE
static constexpr auto LIMINE_DEFAULT_CONFIG = R"(# Limine boot loader configuration
timeout: 5
remember_last_entry: yes

# CachyOS Limine theme
# Author: diegons490 (https://github.com/diegons490/cachyos-limine-theme)
term_palette: 1e1e2e;f38ba8;a6e3a1;f9e2af;89b4fa;f5c2e7;94e2d5;cdd6f4
term_palette_bright: 585b70;f38ba8;a6e3a1;f9e2af;89b4fa;f5c2e7;94e2d5;cdd6f4
term_background: ffffffff
term_foreground: cdd6f4
term_background_bright: ffffffff
term_foreground_bright: cdd6f4
interface_branding:
wallpaper: boot():/limine-splash.png
)"sv;

constexpr auto convert_boolean_val(std::string_view needle, bool value) noexcept -> std::string_view {
    if (needle == "GRUB_ENABLE_CRYPTODISK="sv || needle == "GRUB_DISABLE_SUBMENU="sv) {
        return value ? "y"sv : "n"sv;
    }
    return value ? "true"sv : "false"sv;
}

auto parse_grub_line(const gucc::bootloader::GrubConfig& grub_config, std::string_view line) noexcept -> std::string {
    if (line.starts_with("#GRUB_")) {
        // uncomment grub setting
        line.remove_prefix(1);
    }

    if (line.starts_with("GRUB_")) {
        CONV_REQ_F("GRUB_DEFAULT=", grub_config.default_entry);
        CONV_REQ_F("GRUB_TIMEOUT=", grub_config.grub_timeout);
        CONV_REQ_F_S("GRUB_DISTRIBUTOR=", grub_config.grub_distributor);
        CONV_REQ_F_S("GRUB_CMDLINE_LINUX_DEFAULT=", grub_config.cmdline_linux_default);
        CONV_REQ_F_S("GRUB_CMDLINE_LINUX=", grub_config.cmdline_linux);
        CONV_REQ_F_S("GRUB_PRELOAD_MODULES=", grub_config.preload_modules);
        CONV_REQ_F("GRUB_TIMEOUT_STYLE=", grub_config.timeout_style);
        CONV_REQ_F("GRUB_TERMINAL_INPUT=", grub_config.terminal_input);
        CONV_OPT_F("GRUB_TERMINAL_OUTPUT=", grub_config.terminal_output, "console");
        CONV_REQ_F("GRUB_GFXMODE=", grub_config.gfxmode);
        CONV_REQ_F("GRUB_GFXPAYLOAD_LINUX=", grub_config.gfxpayload_linux);
        CONV_OPT_F_S("GRUB_COLOR_NORMAL=", grub_config.color_normal, "light-blue/black");
        CONV_OPT_F_S("GRUB_COLOR_HIGHLIGHT=", grub_config.color_highlight, "light-cyan/blue");
        CONV_OPT_F_S("GRUB_BACKGROUND=", grub_config.background, "/path/to/wallpaper");
        CONV_OPT_F_S("GRUB_THEME=", grub_config.theme, "/path/to/gfxtheme");
        CONV_OPT_F_S("GRUB_INIT_TUNE=", grub_config.init_tune, "480 440 1");
        if (gucc::utils::contains(line, "=y") || gucc::utils::contains(line, "=true") || gucc::utils::contains(line, "=false")) {
            // booleans
            CONV_OPT_B("GRUB_ENABLE_CRYPTODISK=", grub_config.enable_cryptodisk, "y");
            CONV_OPT_B("GRUB_DISABLE_LINUX_UUID=", grub_config.disable_linux_uuid, "true");
            CONV_REQ_B("GRUB_DISABLE_RECOVERY=", grub_config.disable_recovery);
            CONV_OPT_B("GRUB_SAVEDEFAULT=", grub_config.savedefault, "true");
            CONV_OPT_B("GRUB_DISABLE_SUBMENU=", grub_config.disable_submenu, "y");
            CONV_OPT_B("GRUB_DISABLE_OS_PROBER=", grub_config.disable_os_prober, "false");
        }
    }
    return std::string{line.data(), line.size()};
}

}  // namespace

namespace gucc::bootloader {

auto gen_grub_config(const GrubConfig& grub_config) noexcept -> std::string {
    auto transformed_lines = GRUB_DEFAULT_CONFIG | std::ranges::views::split('\n')
        | std::ranges::views::transform([&](auto&& rng) {
              auto&& line_sv = std::string_view(&*rng.begin(), static_cast<size_t>(std::ranges::distance(rng)));
              return parse_grub_line(grub_config, line_sv);
          });
    std::string result;
    bool first_line = true;
    for (const auto& line_str : transformed_lines) {
        if (!first_line) {
            result += '\n';
        }
        result += line_str;
        first_line = false;
    }
    return result;
}

auto install_systemd_boot(std::string_view root_mountpoint, std::string_view efi_directory, bool is_volume_removable) noexcept -> bool {
    // Install systemd-boot onto EFI
    const auto& bootctl_cmd = fmt::format(FMT_COMPILE("bootctl --path={} install"), efi_directory);
    if (!utils::arch_chroot_checked(bootctl_cmd, root_mountpoint)) {
        spdlog::error("Failed to run bootctl on path {} with: {}", root_mountpoint, bootctl_cmd);
        return false;
    }

    // Generate systemd-boot configuration entries with our sdboot
    static constexpr auto sdboot_cmd = "sdboot-manage gen"sv;
    if (!utils::arch_chroot_checked(sdboot_cmd, root_mountpoint)) {
        spdlog::error("Failed to run sdboot-manage gen on mountpoint: {}", root_mountpoint);
        return false;
    }

    // if the volume is removable don't use autodetect
    if (is_volume_removable) {
        const auto& initcpio_filename = fmt::format(FMT_COMPILE("{}/etc/mkinitcpio.conf"), root_mountpoint);

        // Remove autodetect hook
        auto initcpio = detail::Initcpio{initcpio_filename};
        initcpio.remove_hook("autodetect");
        spdlog::info("\"Autodetect\" hook was removed");
    }
    return true;
}

auto write_grub_config(const GrubConfig& grub_config, std::string_view root_mountpoint) noexcept -> bool {
    const auto& grub_config_content = bootloader::gen_grub_config(grub_config);
    const auto& grub_config_path    = fmt::format(FMT_COMPILE("{}/etc/default/grub"), root_mountpoint);
    if (!file_utils::create_file_for_overwrite(grub_config_path, grub_config_content)) {
        spdlog::error("Failed to open grub config for writing {}", grub_config_path);
        return false;
    }
    return true;
}

auto install_grub(const GrubConfig& grub_config, const GrubInstallConfig& grub_install_config, std::string_view root_mountpoint) noexcept -> bool {
    // Write grub configuration on the system
    if (!bootloader::write_grub_config(grub_config, root_mountpoint)) {
        return false;
    }

    // Get grub install cmd
    const auto& grub_install_cmd = [](const GrubInstallConfig& grub_install_config) -> std::string {
        std::string result{};
        if (grub_install_config.is_root_on_zfs) {
            result += "ZPOOL_VDEV_NAME_PATH=YES "sv;
        }
        result += "grub-install --target="sv;
        result += grub_install_config.is_efi ? "x86_64-efi"sv : "i386-pc"sv;

        if (grub_install_config.do_recheck) {
            result += " --recheck";
        }
        if (grub_install_config.is_removable) {
            result += " --removable";
        }
        if (grub_install_config.is_efi && grub_install_config.efi_directory) {
            result += fmt::format(FMT_COMPILE(" --efi-directory={}"), *grub_install_config.efi_directory);
        }
        if (grub_install_config.is_efi && grub_install_config.bootloader_id) {
            result += fmt::format(FMT_COMPILE(" --bootloader-id={}"), *grub_install_config.bootloader_id);
        }

        // in bios/mbr mode
        if (!grub_install_config.is_efi && grub_install_config.device) {
            result += fmt::format(FMT_COMPILE(" {}"), *grub_install_config.device);
        }
        return result;
    }(grub_install_config);

    // Install grub on the system
    if (!utils::arch_chroot_checked(grub_install_cmd, root_mountpoint)) {
        spdlog::error("Failed to install grub on path {} with: {}", root_mountpoint, grub_install_cmd);
        return false;
    }

    // Generate grub configuration on the boot partition
    static constexpr auto grub_config_cmd = "grub-mkconfig -o /boot/grub/grub.cfg"sv;
    if (!utils::arch_chroot_checked(grub_config_cmd, root_mountpoint)) {
        spdlog::error("Failed to generate grub on path {} with: {}", root_mountpoint, grub_config_cmd);
        return false;
    }

    return true;
}

auto gen_refind_config(const std::vector<std::string>& kernel_params) noexcept -> std::string {
    const auto& kernel_params_str = utils::join(kernel_params, ' ');

    std::string refind_config{};
    refind_config += fmt::format(FMT_COMPILE("\"Boot with standard options\"    \"{}\"\n"), kernel_params_str);
    refind_config += fmt::format(FMT_COMPILE("\"Boot to single-user mode\"    \"{}\" single\n"), kernel_params_str);

    return refind_config;
}

auto refind_write_extra_kern_strings(std::string_view file_path, const std::vector<std::string>& extra_kernel_versions) noexcept -> bool {
    auto&& file_content = file_utils::read_whole_file(file_path);
    if (file_content.empty() || extra_kernel_versions.empty()) {
        return false;
    }

    auto transformed_lines = file_content | std::ranges::views::split('\n')
        | std::ranges::views::transform([&](auto&& rng) {
              /* clang-format off */
              auto&& line_sv = std::string_view(&*rng.begin(), static_cast<size_t>(std::ranges::distance(rng)));
              if (line_sv.starts_with("#extra_kernel_version_strings"sv) || line_sv.starts_with("extra_kernel_version_strings"sv)) {
                  return fmt::format(FMT_COMPILE("extra_kernel_version_strings {}"), utils::join(extra_kernel_versions, ','));
              }
              /* clang-format on */
              return std::string{line_sv.data(), line_sv.size()};
          });
    std::string result;
    bool first_line = true;
    for (const auto& line_str : transformed_lines) {
        if (!first_line) {
            result += '\n';
        }
        result += line_str;
        first_line = false;
    }

    return file_utils::create_file_for_overwrite(file_path, result);
}

auto install_refind(const RefindInstallConfig& refind_install_config) noexcept -> bool {
    // Get refind install cmd
    const auto& refind_install_cmd = [](auto&& root_path, bool is_removable_drive) -> std::string {
        std::string result = fmt::format(FMT_COMPILE("refind-install --root {}"), root_path);
        if (is_removable_drive) {
            result += " --alldrivers --yes";
        }
        result += " &>>/tmp/cachyos-install.log";
        return result;
    }(refind_install_config.root_mountpoint, refind_install_config.is_removable);

    // Install refind on the system
    if (!utils::exec_checked(refind_install_cmd)) {
        spdlog::error("Failed to install refind on path {} with: {}", refind_install_config.root_mountpoint, refind_install_cmd);
        return false;
    }

    // Adjust mkinitcpio for removable drive
    if (refind_install_config.is_removable) {
        const auto& initcpio_filename = fmt::format(FMT_COMPILE("{}/etc/mkinitcpio.conf"), refind_install_config.root_mountpoint);

        // Remove autodetect hook
        auto initcpio = detail::Initcpio{initcpio_filename};
        initcpio.remove_hook("autodetect");
        spdlog::info("'Autodetect' hook was removed");
    }

    // Generate refind configuration
    const auto& refind_config_content = bootloader::gen_refind_config(refind_install_config.kernel_params);

    // Write generated config to system
    const auto& refind_config_path = fmt::format(FMT_COMPILE("{}/boot/refind_linux.conf"), refind_install_config.root_mountpoint);
    if (!file_utils::create_file_for_overwrite(refind_config_path, refind_config_content)) {
        spdlog::error("Failed to open refind config for writing {}", refind_config_path);
        return false;
    }

    // handle extra kernel version strings in {efi boot partition}/EFI/refind/refind.conf
    const auto& extra_refind_config_path = fmt::format(FMT_COMPILE("{}/EFI/refind/refind.conf"), refind_install_config.boot_mountpoint);
    if (!bootloader::refind_write_extra_kern_strings(extra_refind_config_path, refind_install_config.extra_kernel_versions)) {
        spdlog::error("Failed to write extra kernel strings into {}", refind_config_path);
        return false;
    }
    return true;
}

auto gen_limine_config(const std::vector<std::string>& kernel_params) noexcept -> std::string {
    const auto& kernel_params_str = utils::join(kernel_params, ' ');

    std::string limine_config{};
    limine_config += fmt::format(FMT_COMPILE("KERNEL_CMDLINE[default]=\"{}\"\n"), kernel_params_str);

    return limine_config;
}

auto install_limine(const LimineInstallConfig& limine_install_config) noexcept -> bool {
    // Write generated config to system
    const auto& limine_config_path = fmt::format(FMT_COMPILE("{}/limine.conf"), limine_install_config.boot_mountpoint);
    if (!file_utils::create_file_for_overwrite(limine_config_path, LIMINE_DEFAULT_CONFIG)) {
        spdlog::error("Failed to open limine config for writing {}", limine_config_path);
        return false;
    }

    // Write limine-entry-tool config to system
    const auto& limine_entry_tool_config_path = fmt::format(FMT_COMPILE("{}/etc/default/limine"), limine_install_config.root_mountpoint);

    const auto& limine_config_content = bootloader::gen_limine_config(limine_install_config.kernel_params);
    if (!file_utils::create_file_for_overwrite(limine_entry_tool_config_path, limine_config_content)) {
        spdlog::error("Failed to open limine-entry-tool config for writing {}", limine_entry_tool_config_path);
        return false;
    }

    // Install Limine onto EFI
    if (!utils::arch_chroot_checked("limine-install"sv, limine_install_config.root_mountpoint)) {
        spdlog::error("Failed to run limine-install on path {}", limine_install_config.root_mountpoint);
        return false;
    }

    // Add kernel entries
    if (!utils::arch_chroot_checked("limine-mkinitcpio"sv, limine_install_config.root_mountpoint)) {
        spdlog::error("Failed to run limine-mkinitcpio on path {}", limine_install_config.root_mountpoint);
        return false;
    }

    return true;
}

}  // namespace gucc::bootloader
