#include "assets.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace deadtime {
std::string normalize(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    std::vector<std::string> parts;
    std::istringstream input(path);
    for (std::string part; std::getline(input, part, '/');) {
        if (part.empty() || part == ".")
            continue;
        if (part == "..") {
            if (parts.empty())
                throw std::runtime_error("Resource escapes archive root: " + path);
            parts.pop_back();
        } else {
            for (char& c : part)
                if (c >= 'A' && c <= 'Z')
                    c += 'a' - 'A';
            parts.push_back(part);
        }
    }
    std::string result;
    for (const auto& part : parts) {
        if (!result.empty())
            result += '/';
        result += part;
    }
    return result;
}
std::string resolve(const std::string& base, const std::string& path) {
    if (path.empty())
        return {};
    if (path[0] == '/' || path[0] == '\\')
        return normalize(path);
    return normalize(base + '/' + path);
}
std::string directory(const std::string& path) {
    const auto pos = path.find_last_of('/');
    return pos == std::string::npos ? std::string{} : path.substr(0, pos);
}
Assets::~Assets() {
    for (auto* z : archives_)
        zip_close(z);
}
void Assets::mount(const std::filesystem::path& archive) {
    std::map<std::string, Entry> entries;
    if (std::filesystem::is_directory(archive)) {
        const auto root = std::filesystem::canonical(archive);
        for (const auto& item : std::filesystem::recursive_directory_iterator(root)) {
            const auto status = item.symlink_status();
            if (std::filesystem::is_symlink(status))
                throw std::runtime_error("Symlink in resource directory: " + item.path().string());
            if (std::filesystem::is_directory(status))
                continue;
            if (!std::filesystem::is_regular_file(status))
                throw std::runtime_error("Non-regular resource: " + item.path().string());
            const auto relative = item.path().lexically_relative(root).generic_u8string();
            const auto key = normalize(std::string(relative.begin(), relative.end()));
            if (!entries.emplace(key, Entry{nullptr, 0, item.path()}).second)
                throw std::runtime_error("Case-colliding resource: " + key);
        }
        for (auto& [key, entry] : entries)
            index_[key] = std::move(entry);
        return;
    }
    int error = 0;
    auto* z = zip_open(archive.string().c_str(), ZIP_RDONLY, &error);
    if (!z)
        throw std::runtime_error("Cannot open archive: " + archive.string() + " (" + std::to_string(error) +
                                 ")");
    try {
        const auto count = zip_get_num_entries(z, 0);
        if (count < 0)
            throw std::runtime_error("Invalid ZIP directory");
        for (zip_int64_t i = 0; i < count; ++i) {
            const char* n = zip_get_name(z, i, ZIP_FL_ENC_GUESS);
            if (!n)
                throw std::runtime_error("Invalid ZIP directory");
            std::string name(n);
            if (!name.empty() && name.back() != '/') {
                const auto key = normalize(name);
                if (!entries.emplace(key, Entry{z, static_cast<zip_uint64_t>(i), {}}).second)
                    throw std::runtime_error("Case-colliding archive resource: " + key);
            }
        }
    } catch (...) {
        zip_close(z);
        throw;
    }
    archives_.push_back(z);
    for (auto& [key, entry] : entries)
        index_[key] = std::move(entry);
}
bool Assets::isGameRoot(const std::filesystem::path& root) {
    return std::filesystem::is_directory(root / "resources") ||
           std::filesystem::is_regular_file(root / "resources.pak");
}
void Assets::mountGame(const std::filesystem::path& root) {
    for (const auto* layer : {"resources", "localization"}) {
        const auto tree = root / layer;
        const auto pak = root / (std::string(layer) + ".pak");
        if (std::filesystem::is_directory(tree))
            mount(tree);
        else if (std::filesystem::exists(pak) || std::string(layer) == "resources")
            mount(pak);
    }
}
void Assets::put(const std::string& path, std::string bytes) {
    memory_[normalize(path)] = std::move(bytes);
}
bool Assets::contains(const std::string& path) const {
    const auto key = normalize(path);
    return memory_.contains(key) || index_.contains(key);
}
std::string Assets::read(const std::string& path) const {
    const auto key = normalize(path);
    if (auto m = memory_.find(key); m != memory_.end())
        return m->second;
    auto entry = index_.find(key);
    if (entry == index_.end())
        throw std::runtime_error("Missing resource: " + key);
    const auto& [z, i, local] = entry->second;
    if (!local.empty()) {
        // A directory replaced by a link after mounting must not escape the tree.
        if (std::filesystem::canonical(local) != local || !std::filesystem::is_regular_file(local))
            throw std::runtime_error("Resource path changed: " + key);
        const auto size = std::filesystem::file_size(local);
        if (size > 128 * 1024 * 1024)
            throw std::runtime_error("Oversized resource: " + key);
        std::ifstream input(local, std::ios::binary);
        std::string bytes(static_cast<size_t>(size), '\0');
        if (!input.read(bytes.data(), static_cast<std::streamsize>(bytes.size())) ||
            input.peek() != std::char_traits<char>::eof())
            throw std::runtime_error("Cannot read complete resource: " + key);
        return bytes;
    }
    zip_stat_t stat{};
    if (zip_stat_index(z, i, 0, &stat) != 0 || stat.size > 128 * 1024 * 1024)
        throw std::runtime_error("Invalid or oversized resource: " + key);
    auto* file = zip_fopen_index(z, i, 0);
    if (!file)
        throw std::runtime_error("Cannot read resource: " + key);
    std::string result(static_cast<size_t>(stat.size), '\0');
    size_t offset = 0;
    while (offset < result.size()) {
        const auto got = zip_fread(file, result.data() + offset, result.size() - offset);
        if (got <= 0) {
            zip_fclose(file);
            throw std::runtime_error("Truncated resource: " + key);
        }
        offset += static_cast<size_t>(got);
    }
    if (zip_fclose(file) != 0)
        throw std::runtime_error("Resource checksum mismatch: " + key);
    return result;
}
std::vector<std::string> Assets::files(const std::string& extension) const {
    std::vector<std::string> result;
    for (const auto& [name, _] : index_)
        if (name.ends_with(extension))
            result.push_back(name);
    for (const auto& [name, _] : memory_)
        if (name.ends_with(extension))
            result.push_back(name);
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}
} // namespace deadtime
