#include "utils.hpp"
#include "config.hpp"

#include <optional>
#include <vector>
#include <string>

#include <fmt/compile.h>

namespace utils {

auto get_kernel_params() noexcept -> std::optional<std::vector<std::string>> {
    auto* config_instance = Config::instance();
    auto& config_data     = config_instance->data();
    
    const auto& luks     = std::get<std::int32_t>(config_data["LUKS"]);
    const auto& lvm      = std::get<std::int32_t>(config_data["LVM"]);
    const auto& raid     = std::get<std::int32_t>(config_data["RAID"]);
    const auto& partitions = std::get<std::vector<std::string>>(config_data["PARTITIONS"]);
    
    std::vector<std::string> kernel_params;
    
    // Add LUKS parameters
    if (luks == 1) {
        const auto& luks_dev = std::get<std::string>(config_data["LUKS_DEV"]);
        const auto& luks_root_name = std::get<std::string>(config_data["LUKS_ROOT_NAME"]);
        kernel_params.push_back(fmt::format("cryptdevice={}:{}", luks_dev, luks_root_name));
    }
    
    // Add root device
    std::string root_device;
    for (const auto& partition : partitions) {
        if (partition.find("/ ") != std::string::npos || partition.find("/ ext") != std::string::npos || 
            partition.find("/ btrfs") != std::string::npos || partition.find("/ xfs") != std::string::npos) {
            root_device = partition.substr(0, partition.find(' '));
            break;
        }
    }
    
    if (!root_device.empty()) {
        // If using LVM, adjust root device
        if (lvm == 1) {
            const auto& lvm_group = std::get<std::string>(config_data["LVM_GROUP"]);
            root_device = fmt::format("/dev/{}/root", lvm_group);
        }
        // If using LUKS, adjust root device
        else if (luks == 1) {
            const auto& luks_root_name = std::get<std::string>(config_data["LUKS_ROOT_NAME"]);
            root_device = fmt::format("/dev/mapper/{}", luks_root_name);
        }
        
        kernel_params.push_back(fmt::format("root={}", root_device));
    }
    
    // Add other common parameters
    kernel_params.push_back("rw");
    kernel_params.push_back("quiet");
    
    // Add RAID parameters if needed
    if (raid == 1) {
        kernel_params.push_back("rd.auto=1");
    }
    
    return kernel_params;
}

}  // namespace utils
