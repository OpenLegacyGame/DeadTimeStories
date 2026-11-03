#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>
#include <zip.h>

namespace deadtime {
// Paths inside the original archives follow Windows case-insensitive rules.
std::string normalize(std::string path);
std::string resolve(const std::string& base, const std::string& path);
std::string directory(const std::string& path);

class Assets {
  public:
    Assets() = default;
    ~Assets();
    Assets(const Assets&) = delete;
    Assets& operator=(const Assets&) = delete;
    // Later mounts overlay earlier ones. Directories are read lazily for editing.
    void mount(const std::filesystem::path& archiveOrDirectory);
    void mountGame(const std::filesystem::path& root);
    static bool isGameRoot(const std::filesystem::path& root);
    // In-memory mounting supports small, independently authored test fixtures.
    void put(const std::string& path, std::string bytes);
    bool contains(const std::string& path) const;
    std::string read(const std::string& path) const;
    std::vector<std::string> files(const std::string& extension) const;

  private:
    struct Entry {
        zip_t* archive = nullptr;
        zip_uint64_t index = 0;
        std::filesystem::path file;
    };
    std::vector<zip_t*> archives_;
    std::map<std::string, Entry> index_;
    std::map<std::string, std::string> memory_;
};
} // namespace deadtime
