#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
namespace deadtime {
// A file belongs to a persistent profile ID, independent of its display name.
class GameSaves {
  public:
    explicit GameSaves(std::filesystem::path directory) : directory_(std::move(directory)) {}
    std::optional<std::string> load(const std::string& profile) const;
    void save(const std::string& profile, const std::string& checkpoint) const;
    void newGame(const std::string& profile) const;
    void discard(const std::string& profile) const;
    std::filesystem::path path(const std::string& profile) const;

  private:
    std::filesystem::path directory_;
    void write(const std::string& profile, const std::optional<std::string>& checkpoint) const;
};
} // namespace deadtime
