#include "hob.hpp"
#include "particles.hpp"
#include <algorithm>
#include <iterator>
#include <map>
#include <pugixml.hpp>
#include <stdexcept>
#include <string_view>

namespace deadtime {
namespace {
std::string label(std::string value) {
    if (value.size() >= 2 && value.front() == '%' && value.back() == '%')
        return value.substr(1, value.size() - 2);
    return value;
}
HobPiece readPiece(pugi::xml_node node, const std::string& base) {
    HobPiece result;
    result.texture = resolve(base, node.child("texture").attribute("src").value());
    auto p = node.child("position");
    result.x = p.attribute("x").as_float();
    result.y = p.attribute("y").as_float();
    result.z = p.attribute("z").as_float();
    auto size = node.child("size");
    result.width = size.attribute("width").as_float();
    result.height = size.attribute("height").as_float();
    result.alpha = node.child("alpha").attribute("value").as_float(1);
    result.visible = node.child("visible").attribute("value").as_bool(true);
    result.interactive = node.child("interactive").attribute("value").as_bool(true);
    if (const auto inventory = node.child("inventory")) {
        result.inventory = inventory.attribute("name").value();
        result.inventoryMessage = inventory.attribute("message").value();
        result.dropInventory = inventory.attribute("drop").as_bool();
        result.removeInventory = inventory.attribute("remove").as_bool();
    }
    return result;
}
} // namespace
bool HobItem::found() const {
    return std::all_of(pieces.begin(), pieces.end(), [](auto& p) { return p.found; });
}
size_t HobItem::remaining() const {
    return static_cast<size_t>(
        std::count_if(pieces.begin(), pieces.end(), [](const auto& piece) { return !piece.found; }));
}
std::string HobItem::listText() const {
    const auto count = remaining();
    return count > 1 ? name + " (" + std::to_string(count) + ")" : name;
}
HiddenObjectRound::HiddenObjectRound(Assets& assets, const std::string& path, const std::string& round,
                                     uint32_t seed) {
    pugi::xml_document doc;
    const auto bytes = assets.read(path);
    auto parsed = doc.load_buffer(bytes.data(), bytes.size());
    if (!parsed)
        throw std::runtime_error(path + ": " + parsed.description());
    const auto root = doc.child("hob");
    const auto base = directory(path);
    std::map<std::string, std::map<std::string, size_t>> groups;
    for (auto group : root.child("groups").children()) {
        for (auto node : group.children()) {
            HobItem item;
            item.name = label(node.attribute("name").value());
            item.environment = group.attribute("environment").as_bool(true);
            if (std::string(node.name()) == "complex") {
                for (auto piece : node.children("object"))
                    item.pieces.push_back(readPiece(piece, base));
            } else if (std::string(node.name()) == "object")
                item.pieces.push_back(readPiece(node, base));
            else
                throw std::runtime_error("Unsupported hidden-object entry: " + std::string(node.name()));
            if (item.pieces.empty())
                throw std::runtime_error("Empty hidden object: " + item.name);
            if (!groups[group.name()].emplace(node.attribute("name").value(), items.size()).second)
                throw std::runtime_error("Duplicate hidden object: " + item.name);
            items.push_back(std::move(item));
        }
    }
    for (auto node : root.child("environment").children("object"))
        background.push_back(readPiece(node, base));
    const auto definition = root.child("rounds").child(round.c_str());
    if (!definition)
        throw std::runtime_error("Unknown hidden-object round: " + path + ':' + round);
    // The original round walks a name-ordered map and draws once per random slot.
    ParticleRandom random(seed);
    for (auto slot : definition.children()) {
        const auto groupName = slot.attribute("group").as_string();
        auto group = groups.find(groupName);
        if (group == groups.end())
            throw std::runtime_error("Unknown hidden-object group: " + std::string(groupName));
        const std::string_view exact = slot.attribute("object").as_string();
        auto& candidates = group->second;
        if (candidates.empty())
            throw std::runtime_error("No distinct item for hidden-object slot: " + std::string(slot.name()));
        auto entry = exact.empty() ? std::next(candidates.begin(), random.next() % candidates.size())
                                   : candidates.find(std::string(exact));
        if (!exact.empty() && entry == candidates.end()) {
            const auto displayed = label(std::string(exact));
            entry = std::find_if(candidates.begin(), candidates.end(), [&](const auto& candidate) {
                return items[candidate.second].name == displayed;
            });
        }
        if (entry == candidates.end())
            throw std::runtime_error("Missing fixed hidden object: " + std::string(exact));
        const auto chosen = entry->second;
        candidates.erase(entry);
        items[chosen].target = true;
        targets.push_back(chosen);
    }
    if (targets.empty())
        throw std::runtime_error("Empty hidden-object round");
    auto script = [&](const char* name) {
        const auto attr = definition.attribute(name);
        return attr ? resolve(base, attr.value()) + ".lua" : std::string{};
    };
    enterScript = script("script_enter");
    completeScript = script("script_complete");
    congratulations = definition.attribute("congratulations").value();
    rewardImage = resolve(base, definition.attribute("congratulations_image").value());
}
bool HiddenObjectRound::pick(size_t item, size_t piece, const std::string& inventory) {
    if (item >= items.size() || piece >= items[item].pieces.size())
        return false;
    auto& target = items[item];
    if (!target.target || target.pieces[piece].found || rewarded)
        return false;
    if (target.pieces[piece].inventory != inventory)
        return false;
    target.pieces[piece].found = true;
    return true;
}
bool HiddenObjectRound::complete() const {
    return std::all_of(targets.begin(), targets.end(), [&](auto index) { return items[index].found(); });
}
} // namespace deadtime
