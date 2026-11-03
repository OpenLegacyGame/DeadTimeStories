#include "settings.hpp"
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#endif
namespace deadtime {
namespace {
constexpr const char* names[] = {"effects", "environment", "music"};
void require(bool value, const char* message) {
    if (!value)
        throw std::runtime_error(message);
}
} // namespace
Settings::Settings(std::filesystem::path file) : file_(std::move(file)) {
    if (file_.empty() || !std::filesystem::exists(file_))
        return;
    require(std::filesystem::file_size(file_) < 4096, "Settings file is too large");
    pugi::xml_document doc;
    require(bool(doc.load_file(file_.c_str())), "Cannot read settings");
    auto root = doc.child("deadtime_settings");
    require(root && !root.next_sibling() && std::string(root.attribute("version").value()) == "1",
            "Unknown settings format");
    for (size_t i = 0; i < volumes_.size(); ++i) {
        const auto attribute = root.attribute(names[i]);
        const auto text = std::string(attribute.value());
        size_t consumed = 0;
        require(attribute && !text.empty(), "Missing settings volume");
        const float value = std::stof(text, &consumed);
        require(consumed == text.size() && std::isfinite(value) && value >= 0 && value <= 1,
                "Invalid settings volume");
        volumes_[i] = value;
    }
    const auto mode = std::string(root.attribute("windowed").value());
    require(mode == "0" || mode == "1", "Invalid settings display mode");
    windowed_ = mode == "1";
}
void Settings::volume(AudioBus bus, float value) {
    require(std::isfinite(value) && value >= 0 && value <= 1, "Invalid volume");
    volumes_.at(static_cast<size_t>(bus)) = value;
}
void Settings::save() const {
    if (file_.empty())
        return;
    pugi::xml_document doc;
    auto root = doc.append_child("deadtime_settings");
    root.append_attribute("version") = 1;
    for (size_t i = 0; i < volumes_.size(); ++i)
        root.append_attribute(names[i]) = volumes_[i];
    root.append_attribute("windowed") = windowed_ ? 1 : 0;
    if (!file_.parent_path().empty())
        std::filesystem::create_directories(file_.parent_path());
    auto temporary = file_;
    temporary += ".tmp";
    require(doc.save_file(temporary.c_str(), "", pugi::format_raw, pugi::encoding_utf8),
            "Cannot write settings");
#ifdef _WIN32
    require(MoveFileExW(temporary.c_str(), file_.c_str(),
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0,
            "Cannot replace settings");
#else
    std::filesystem::rename(temporary, file_);
#endif
}
} // namespace deadtime
