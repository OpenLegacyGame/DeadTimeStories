#include "profiles.hpp"
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <pugixml.hpp>
#include <random>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#endif
namespace deadtime {
Profiles::Profiles(const Assets& assets, std::filesystem::path file) : file_(std::move(file)) {
    auto bytes = assets.read("menu/profiles/profiles.cfg");
    pugi::xml_document config;
    if (!config.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid profiles configuration");
    alphabet_ = config.child("profiles").child_value("profile_alphabet");
    if (alphabet_.empty())
        throw std::runtime_error("Empty profile alphabet");
    if (!std::filesystem::exists(file_))
        return;
    pugi::xml_document document;
    const auto loaded = document.load_file(file_.c_str());
    if (!loaded || !document.child("profiles"))
        throw std::runtime_error("Cannot read user profiles: " + file_.string());
    auto root = document.child("profiles");
    const std::string version = root.attribute("version").as_string();
    if (!version.empty() && version != "1")
        throw std::runtime_error("Unsupported user profile format");
    const bool legacy = version.empty();
    for (auto node : root.children("profile")) {
        const std::string name = node.attribute("name").as_string();
        if (!canCreate(name))
            throw std::runtime_error("Invalid or duplicate stored user profile");
        const std::string id = node.attribute("id").as_string();
        if (legacy) {
            if (node.attribute("id"))
                throw std::runtime_error("Invalid legacy profile ID");
        } else if (id.size() != 32 || id.find_first_not_of("0123456789abcdef") != std::string::npos ||
                   std::find(ids_.begin(), ids_.end(), id) != ids_.end())
            throw std::runtime_error("Invalid or duplicate user profile ID");
        names_.push_back(name);
        ids_.push_back(id);
    }
    if (!names_.empty()) {
        const std::string selected = root.attribute("current").as_string();
        const auto found = std::find(names_.begin(), names_.end(), selected);
        if (found == names_.end())
            throw std::runtime_error("Stored current user does not exist");
        current_ = size_t(found - names_.begin());
    }
    if (legacy && !names_.empty()) {
        for (auto& id : ids_)
            id = newId();
        save(names_, ids_, current_);
    }
}
bool Profiles::canCreate(const std::string& name) const {
    return names_.size() < 5 && !name.empty() && name.find_first_not_of(alphabet_) == std::string::npos &&
           std::find(names_.begin(), names_.end(), name) == names_.end();
}
std::string Profiles::newId() const {
    std::random_device random;
    for (;;) {
        std::ostringstream output;
        output << std::hex << std::setfill('0');
        for (unsigned i = 0; i < 4; ++i)
            output << std::setw(8) << uint32_t(random());
        auto id = output.str();
        if (std::find(ids_.begin(), ids_.end(), id) == ids_.end())
            return id;
    }
}
void Profiles::save(const std::vector<std::string>& names, const std::vector<std::string>& ids,
                    std::optional<size_t> current) const {
    pugi::xml_document document;
    auto root = document.append_child("profiles");
    root.append_attribute("version") = 1;
    if (current)
        root.append_attribute("current") = names.at(*current).c_str();
    for (size_t i = 0; i < names.size(); ++i) {
        auto node = root.append_child("profile");
        node.append_attribute("name") = names[i].c_str();
        node.append_attribute("id") = ids.at(i).c_str();
    }
    if (!file_.parent_path().empty())
        std::filesystem::create_directories(file_.parent_path());
    auto temporary = file_;
    temporary += ".tmp";
    if (!document.save_file(temporary.c_str(), "  ", pugi::format_default, pugi::encoding_utf8))
        throw std::runtime_error("Cannot write user profiles");
    // Replace without deleting the previous valid file first. A write or
    // replacement failure leaves both the in-memory and old disk state intact.
#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), file_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        throw std::runtime_error("Cannot replace user profiles");
#else
    std::filesystem::rename(temporary, file_);
#endif
}
bool Profiles::create(const std::string& name) {
    if (!canCreate(name))
        return false;
    auto next = names_;
    next.push_back(name);
    auto ids = ids_;
    ids.push_back(newId());
    save(next, ids, next.size() - 1);
    names_ = std::move(next);
    ids_ = std::move(ids);
    current_ = names_.size() - 1;
    return true;
}
bool Profiles::select(size_t index) {
    if (index >= names_.size())
        return false;
    if (current_ == index)
        return true;
    save(names_, ids_, index);
    current_ = index;
    return true;
}
bool Profiles::erase(size_t index) {
    if (names_.size() <= 1 || index >= names_.size())
        return false;
    auto next = names_;
    next.erase(next.begin() + index);
    auto ids = ids_;
    ids.erase(ids.begin() + index);
    auto selected = current_.value_or(0);
    if (selected > index)
        --selected;
    else if (selected == index)
        selected = std::min(index, next.size() - 1);
    save(next, ids, selected);
    names_ = std::move(next);
    ids_ = std::move(ids);
    current_ = selected;
    return true;
}
ProfileMenu::ProfileMenu(Assets& assets, Profiles& profiles)
    : profiles_(profiles), list_(loadGui(assets, "menu/profiles/profiles_list.gui")),
      create_(loadGui(assets, "menu/profiles/profiles_create.gui")),
      confirmation_(loadGui(assets, "menu/delete_profile/layout.gui")) {
    open();
}
void ProfileMenu::open() {
    closed_ = false;
    deleting_.reset();
    input_.clear();
    first_ = profiles_.names().empty();
    view_ = first_ ? View::Create : View::List;
}
bool ProfileMenu::text(const std::string& input) {
    if (view_ != View::Create || input.find_first_not_of(profiles_.alphabet()) != std::string::npos)
        return false;
    input_ += input;
    return true;
}
bool ProfileMenu::backspace() {
    if (view_ != View::Create || input_.empty())
        return false;
    input_.pop_back();
    return true;
}
void ProfileMenu::navigate(int direction) {
    if (view_ != View::List || !profiles_.current())
        return;
    const auto index = int(*profiles_.current()) + direction;
    if (index >= 0 && size_t(index) < profiles_.names().size())
        profiles_.select(size_t(index));
}
void ProfileMenu::action(const std::string& id) {
    if (view_ == View::Delete) {
        if (id == "yes" && deleting_) {
            profiles_.erase(*deleting_);
            deleting_.reset();
            view_ = View::List;
        } else if (id == "no") {
            deleting_.reset();
            view_ = View::List;
        }
    } else if (view_ == View::Create) {
        if (id == "create" && profiles_.create(input_)) {
            input_.clear();
            view_ = View::List;
            closed_ = first_;
        } else if (id == "cancel" && !profiles_.names().empty())
            view_ = View::List;
    } else if (id == "create" && profiles_.names().size() < 5) {
        view_ = View::Create;
        input_.clear();
    } else if (id == "exit")
        closed_ = true;
    else if (id == "navigate_up")
        navigate(-1);
    else if (id == "navigate_down")
        navigate(1);
    else if (id == "delete_shortcut" && profiles_.names().size() > 1) {
        deleting_ = profiles_.current();
        view_ = View::Delete;
    } else
        for (size_t i = 0; i < profiles_.names().size(); ++i) {
            if (id == "item_back" + std::to_string(i))
                profiles_.select(i);
            else if (id == "item_ok" + std::to_string(i)) {
                profiles_.select(i);
                closed_ = true;
            } else if (id == "item_del" + std::to_string(i) && profiles_.current() == i &&
                       profiles_.names().size() > 1) {
                deleting_ = i;
                view_ = View::Delete;
            }
        }
}
std::vector<GuiElement> ProfileMenu::elements() const {
    auto result = view_ == View::Create ? create_ : view_ == View::Delete ? confirmation_ : list_;
    for (auto& element : result) {
        if (view_ == View::Create) {
            if (element.id == "edit")
                element.text = input_;
            if (element.id == "create")
                element.enabled = profiles_.canCreate(input_);
            if (element.id == "cancel")
                element.enabled = !profiles_.names().empty();
        } else if (view_ == View::List) {
            if (element.id == "create")
                element.enabled = profiles_.names().size() < 5;
            for (size_t i = 0; i < 5; ++i) {
                const auto suffix = std::to_string(i);
                const bool exists = i < profiles_.names().size(), selected = profiles_.current() == i;
                if (element.id == "empty_label" + suffix)
                    element.visible = !exists;
                if (element.id == "item_label" + suffix) {
                    element.visible = exists;
                    element.text = exists ? profiles_.names()[i] : "";
                }
                if (element.id == "item_back" + suffix)
                    element.visible = exists;
                if (element.id == "item_selector" + suffix)
                    element.visible = exists && selected;
                if (element.id == "item_del" + suffix) {
                    element.visible = exists && selected;
                    element.enabled = profiles_.names().size() > 1;
                }
                if (element.id == "item_ok" + suffix)
                    element.visible = exists && selected;
            }
        }
    }
    return result;
}
} // namespace deadtime
