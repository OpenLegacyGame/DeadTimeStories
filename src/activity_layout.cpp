#include "activity_layout.hpp"
#include "animation.hpp"
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>
namespace deadtime {
namespace {
std::pair<float, float> jpegSize(const std::string& bytes) {
    auto byte = [&](size_t at) { return static_cast<unsigned char>(bytes.at(at)); };
    auto word = [&](size_t at) { return unsigned(byte(at)) * 256 + byte(at + 1); };
    if (bytes.size() < 4 || byte(0) != 0xFF || byte(1) != 0xD8)
        throw std::runtime_error("Expected JPEG activity texture");
    for (size_t at = 2; at + 3 < bytes.size();) {
        if (byte(at++) != 0xFF)
            break;
        while (at < bytes.size() && byte(at) == 0xFF)
            ++at;
        if (at >= bytes.size())
            break;
        const auto marker = byte(at++);
        if (marker == 0xD9 || marker == 0xDA)
            break;
        if (marker == 0xD8 || (marker >= 0xD0 && marker <= 0xD7) || marker == 1)
            continue;
        if (at + 1 >= bytes.size())
            break;
        const auto length = word(at);
        if (length < 2 || length > bytes.size() - at)
            break;
        if (marker >= 0xC0 && marker <= 0xCF && marker != 0xC4 && marker != 0xC8 && marker != 0xCC) {
            if (length < 8 || word(at + 3) == 0 || word(at + 5) == 0)
                break;
            return {float(word(at + 5)), float(word(at + 3))};
        }
        at += length;
    }
    throw std::runtime_error("Missing JPEG activity dimensions");
}
constexpr double radians = 3.14159265358979323846 / 180;
std::pair<float, float> rotate(float x, float y, float angle) {
    const auto c = std::cos(angle * radians), s = std::sin(angle * radians);
    return {float(x * c - y * s), float(x * s + y * c)};
}
} // namespace
void measureActivityTextures(const Assets& assets, std::vector<ActivityObject>& objects) {
    for (auto& object : objects)
        if (!object.texture.empty() && (object.width <= 0 || object.height <= 0)) {
            const auto resource = textureResource(assets, object.texture);
            if (resource.empty())
                throw std::runtime_error("Missing activity texture: " + object.texture);
            const auto [width, height] = jpegSize(assets.read(resource));
            if (object.width <= 0)
                object.width = width;
            if (object.height <= 0)
                object.height = height;
        }
}
bool ActivityObject::contains(float mouseX, float mouseY) const {
    if (scale <= 0)
        return false;
    const auto local = rotate(mouseX - x, mouseY - y, -angle);
    const auto u = local.first / scale + pivotX, v = local.second / scale + pivotY;
    return u >= 0 && v >= 0 && u < width && v < height;
}
std::vector<ActivityObject> loadActivityLayout(const Assets& assets, const std::string& path) {
    const auto base = directory(path);
    const auto bytes = assets.read(path);
    pugi::xml_document doc;
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid activity layout: " + path);
    std::vector<ActivityObject> objects;
    auto root = doc.child("objects");
    if (!root)
        root = doc.child("cutscene").child("objects");
    if (!root)
        throw std::runtime_error("Missing activity objects: " + path);
    for (auto node : root.children("object")) {
        ActivityObject object;
        object.id = node.attribute("name").value();
        object.texture = resolve(base, node.child("texture").attribute("src").value());
        const auto position = node.child("position");
        object.x = position.attribute("x").as_float();
        object.y = position.attribute("y").as_float();
        object.z = position.attribute("z").as_float();
        object.width = node.child("size").attribute("width").as_float();
        object.height = node.child("size").attribute("height").as_float();
        object.angle = node.child("rotationZ").attribute("degrees").as_float();
        object.alpha = node.child("alpha").attribute("value").as_float(1);
        object.scale = node.child("scale").attribute("value").as_float(1);
        object.pivotX = node.child("pivot").attribute("x").as_float();
        object.pivotY = node.child("pivot").attribute("y").as_float();
        object.additive =
            resolve(base, node.child("material").attribute("src").value()) == "materials/quad_tex_add";
        if (node.child("enable").attribute("value") && !node.child("enable").attribute("value").as_bool())
            object.enabled = false;
        object.visible = node.child("visible").attribute("value").as_bool(true);
        object.interactive = node.child("interactive").attribute("value").as_bool(true);
        objects.push_back(std::move(object));
    }
    return objects;
}
} // namespace deadtime
