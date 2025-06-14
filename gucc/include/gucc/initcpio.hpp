#ifndef INITCPIO_HPP
#define INITCPIO_HPP

#include <algorithm>    // for find, contains
#include <ranges>       // for ranges::*
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace gucc::detail {

class Initcpio final {
 public:
    explicit Initcpio(const std::string_view& file_path) noexcept
      : m_file_path(file_path) { }

    bool parse_file() noexcept;

    inline bool append_module(std::string&& module) noexcept {
        /* clang-format off */
        if (!this->parse_file()) { return false; }
        if (std::ranges::contains(modules, module)) { return false; }
        /* clang-format on */

        modules.emplace_back(std::move(module));
        return this->write();
    }
    inline bool append_file(std::string&& file) noexcept {
        /* clang-format off */
        if (!this->parse_file()) { return false; }
        if (std::ranges::contains(files, file)) { return false; }
        /* clang-format on */

        files.emplace_back(std::move(file));
        return this->write();
    }
    inline bool append_hook(std::string&& hook) noexcept {
        /* clang-format off */
        if (!this->parse_file()) { return false; }
        if (std::ranges::contains(hooks, hook)) { return false; }
        /* clang-format on */

        hooks.emplace_back(std::move(hook));
        return this->write();
    }
    inline bool append_hooks(std::vector<std::string>&& hook) noexcept {
        /* clang-format off */
        if (!this->parse_file()) { return false; }
        auto filtered_elements = hook | std::ranges::views::filter([&](auto&& el) {
            return !std::ranges::contains(hooks, el);
        });
        std::vector<std::string> to_add(filtered_elements.begin(), filtered_elements.end());
        if (to_add.empty()) { return false; }

        hooks.insert(hooks.end(), to_add.begin(), to_add.end());
        return this->write();
    }
    inline bool insert_hook(std::string&& needle, std::string&& hook) noexcept {
        /* clang-format off */
        if (!this->parse_file()) { return false; }
        if (std::ranges::contains(hooks, hook)) { return false; }
        /* clang-format on */

        auto&& needle_pos = std::ranges::find(hooks, std::move(needle));
        hooks.insert(std::move(needle_pos), std::move(hook));
        return this->write();
    }
    inline bool insert_hook(std::string&& needle, std::vector<std::string>&& hook) noexcept {
        /* clang-format off */
        if (!this->parse_file()) { return false; }
        auto filtered_elements = hook | std::ranges::views::filter([&](auto&& el) {
            return !std::ranges::contains(hooks, el);
        });
        std::vector<std::string> to_add(filtered_elements.begin(), filtered_elements.end());
        if (to_add.empty()) { return false; }
        /* clang-format on */

        auto needle_pos = std::ranges::find(hooks, needle); // Pass needle by const ref or value if not meant to be moved yet
        hooks.insert(needle_pos, to_add.begin(), to_add.end());
        return this->write();
    }

    inline bool remove_module(std::string&& module) noexcept {
        /* clang-format off */
        if (!this->parse_file()) { return false; }
        auto&& needle_pos = std::ranges::find(modules, std::move(module));
        if (needle_pos == modules.end()) { return false; }
        /* clang-format on */

        modules.erase(std::move(needle_pos));
        return this->write();
    }
    inline bool remove_hook(std::string&& hook) noexcept {
        /* clang-format off */
        if (!this->parse_file()) { return false; }
        auto&& needle_pos = std::ranges::find(hooks, std::move(hook));
        if (needle_pos == hooks.end()) { return false; }
        /* clang-format on */

        hooks.erase(std::move(needle_pos));
        return this->write();
    }

    [[nodiscard]] bool write() const noexcept;

    std::vector<std::string> modules{};
    std::vector<std::string> files{};
    std::vector<std::string> hooks{};

 private:
    std::string_view m_file_path{};
};

}  // namespace gucc::detail

#endif  // INITCPIO_HPP
