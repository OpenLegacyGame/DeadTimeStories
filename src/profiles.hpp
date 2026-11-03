#pragma once
#include "gui.hpp"
#include <filesystem>
#include <optional>
namespace deadtime {
// Persistent user identities. Gameplay checkpoints are stored separately.
class Profiles {
  public:
    Profiles(const Assets& assets, std::filesystem::path file);
    bool canCreate(const std::string& name) const;
    bool create(const std::string& name);
    bool select(size_t index);
    bool erase(size_t index);
    const auto& names() const {
        return names_;
    }
    const auto& ids() const {
        return ids_;
    }
    std::optional<size_t> current() const {
        return current_;
    }
    const auto& alphabet() const {
        return alphabet_;
    }

  private:
    std::filesystem::path file_;
    std::string alphabet_;
    std::vector<std::string> names_;
    std::vector<std::string> ids_;
    std::optional<size_t> current_;
    void save(const std::vector<std::string>& names, const std::vector<std::string>& ids,
              std::optional<size_t> current) const;
    std::string newId() const;
};
class ProfileMenu {
  public:
    enum class View { List, Create, Delete };
    ProfileMenu(Assets& assets, Profiles& profiles);
    void open();
    bool text(const std::string& input);
    bool backspace();
    void action(const std::string& id);
    void navigate(int direction);
    std::vector<GuiElement> elements() const;
    View view() const {
        return view_;
    }
    bool closed() const {
        return closed_;
    }
    const auto& input() const {
        return input_;
    }

  private:
    Profiles& profiles_;
    View view_ = View::List;
    bool closed_ = false;
    bool first_ = false;
    std::string input_;
    std::optional<size_t> deleting_;
    std::vector<GuiElement> list_, create_, confirmation_;
};
} // namespace deadtime
