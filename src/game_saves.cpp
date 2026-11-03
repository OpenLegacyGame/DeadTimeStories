#include "game_saves.hpp"
#include <fstream>
#include <pugixml.hpp>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#endif
namespace deadtime {
namespace {
void require(bool ok, const char* message) {
    if (!ok)
        throw std::runtime_error(message);
}
} // namespace
std::filesystem::path GameSaves::path(const std::string& profile) const {
    require(profile.size() == 32 && profile.find_first_not_of("0123456789abcdef") == std::string::npos,
            "Invalid progress profile ID");
    return directory_ / (profile + ".xml");
}
std::optional<std::string> GameSaves::load(const std::string& profile) const {
    const auto file = path(profile);
    if (!std::filesystem::exists(file))
        return {};
    require(std::filesystem::file_size(file) <= 33 * 1024 * 1024, "Game progress file is too large");
    pugi::xml_document document;
    require(bool(document.load_file(file.c_str(), pugi::parse_default | pugi::parse_ws_pcdata_single)),
            "Cannot read game progress");
    const auto root = document.child("game_progress");
    require(root && !root.next_sibling("game_progress") &&
                std::string(root.attribute("version").value()) == "1" &&
                profile == root.attribute("profile").value(),
            "Game progress belongs to another profile or format");
    const std::string status = root.attribute("status").value();
    const auto saved = root.child("deadtime_checkpoint");
    if (status == "fresh") {
        require(!saved, "Fresh game contains conflicting progress");
        return {};
    }
    require(status == "saved" && saved && !saved.next_sibling("deadtime_checkpoint"),
            "Missing or duplicate game checkpoint");
    std::ostringstream output;
    saved.print(output, "  ", pugi::format_default, pugi::encoding_utf8);
    return output.str();
}
void GameSaves::write(const std::string& profile, const std::optional<std::string>& checkpoint) const {
    const auto file = path(profile);
    pugi::xml_document document;
    auto root = document.append_child("game_progress");
    root.append_attribute("version") = 1;
    root.append_attribute("profile") = profile.c_str();
    root.append_attribute("status") = checkpoint ? "saved" : "fresh";
    if (checkpoint) {
        require(checkpoint->size() <= 32 * 1024 * 1024, "Game checkpoint is too large");
        pugi::xml_document source;
        require(bool(source.load_buffer(checkpoint->data(), checkpoint->size(),
                                        pugi::parse_default | pugi::parse_ws_pcdata_single)),
                "Invalid game checkpoint XML");
        auto saved = source.child("deadtime_checkpoint");
        require(saved && !saved.next_sibling("deadtime_checkpoint") && saved.attribute("version") &&
                    saved.attribute("resources"),
                "Missing game checkpoint header");
        root.append_copy(saved);
    }
    std::filesystem::create_directories(directory_);
    auto temporary = file;
    temporary += ".tmp";
    // Avoid adding another indentation level to every checkpoint line.
    require(document.save_file(temporary.c_str(), "", pugi::format_raw, pugi::encoding_utf8),
            "Cannot write game progress");
#ifdef _WIN32
    require(
        MoveFileExW(temporary.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0,
        "Cannot replace game progress");
#else
    std::filesystem::rename(temporary, file);
#endif
}
void GameSaves::save(const std::string& profile, const std::string& checkpoint) const {
    write(profile, checkpoint);
}
void GameSaves::newGame(const std::string& profile) const {
    write(profile, {});
}
void GameSaves::discard(const std::string& profile) const {
    std::filesystem::remove(path(profile));
}
} // namespace deadtime
