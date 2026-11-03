#include "gui.hpp"
#include <functional>
#include <pugixml.hpp>
#include <stdexcept>

namespace deadtime {
std::vector<GuiElement> loadGui(Assets& assets, const std::string& path,
                                const std::map<std::string, bool>& visibility) {
    std::vector<GuiElement> result;
    std::function<void(const std::string&, float, float, bool, int, const std::vector<std::string>&)> include;
    std::function<void(pugi::xml_node, const std::string&, float, float, bool, int,
                       const std::vector<std::string>&)>
        visit;
    std::function<void(GuiElement&, pugi::xml_node, const std::string&, int)> properties;
    properties = [&](GuiElement& element, pugi::xml_node node, const std::string& base, int depth) {
        if (depth > 32)
            throw std::runtime_error("Recursive GUI template");
        if (auto attribute = node.attribute("template")) {
            const std::string reference = attribute.value();
            const auto separator = reference.find('#');
            if (separator == std::string::npos)
                throw std::runtime_error("Invalid GUI template: " + reference);
            const auto file = resolve(base, reference.substr(0, separator));
            pugi::xml_document doc;
            const auto bytes = assets.read(file);
            if (!doc.load_buffer(bytes.data(), bytes.size()))
                throw std::runtime_error("Invalid template XML: " + reference);
            const auto definition = doc.child("templates").child(reference.substr(separator + 1).c_str());
            if (!definition)
                throw std::runtime_error("Missing GUI template: " + reference);
            properties(element, definition, directory(file), depth + 1);
        }
        auto number = [](pugi::xml_attribute value, float& destination) {
            if (value)
                destination = value.as_float();
        };
        number(node.child("position").attribute("x"), element.x);
        number(node.child("position").attribute("y"), element.y);
        number(node.child("mask").attribute("width"), element.width);
        number(node.child("mask").attribute("height"), element.height);
        number(node.child("size").attribute("width"), element.width);
        number(node.child("size").attribute("height"), element.height);
        number(node.child("slideline").child("start").attribute("x"), element.slideStartX);
        number(node.child("slideline").child("start").attribute("y"), element.slideStartY);
        number(node.child("slideline").child("end").attribute("x"), element.slideEndX);
        number(node.child("slideline").child("end").attribute("y"), element.slideEndY);
        if (auto value = node.attribute("visible"))
            element.visible = value.as_bool();
        if (auto value = node.attribute("transparent"))
            element.transparent = value.as_bool();
        if (auto value = node.attribute("shortcuts"))
            element.shortcuts = value.value();
        for (auto target : node.child("influence").child("hover").child("onEnter").children())
            if (auto value = target.attribute("text_color"))
                element.hoverTextColors[target.name()] = unsigned(std::stoul(value.value(), nullptr, 16));
        for (auto target : node.child("influence").child("hover").child("onEnter").children())
            if (auto value = target.attribute("texture"))
                element.hoverTextures[target.name()] = resolve(base, value.value());
        if (auto value = node.child("align").attribute("hor"))
            element.horizontalAlign = value.value();
        auto vertical = node.child("align").attribute("ver");
        if (!vertical)
            vertical = node.child("align").attribute("vert");
        if (vertical)
            element.verticalAlign = vertical.value();
        auto resource = [&](pugi::xml_node child, std::string& destination) {
            if (auto value = child.attribute("src"))
                destination = resolve(base, value.value());
        };
        resource(node.child("file"), element.normal);
        if (auto view = node.child("view")) {
            resource(view.child("normal"), element.normal);
            resource(view.child("hover"), element.hover);
            resource(view.child("clicked"), element.clicked);
            resource(view.child("disabled"), element.disabled);
            resource(view.child("normal_checked"), element.normalChecked);
            resource(view.child("hover_checked"), element.hoverChecked);
            resource(view.child("clicked_checked"), element.clickedChecked);
            for (auto frame : view.children("file"))
                element.progress[frame.attribute("progress").as_float()] =
                    resolve(base, frame.attribute("src").value());
        }
        resource(node.child("mask"), element.mask);
        if (auto text = node.child("text")) {
            if (text.first_child())
                element.text = text.text().as_string();
            if (auto value = text.attribute("font"))
                element.font = resolve(base, value.value());
            if (auto value = text.attribute("color"))
                element.color = static_cast<unsigned>(std::stoul(value.value(), nullptr, 16));
        }
    };
    include = [&](const std::string& file, float x, float y, bool visible, int depth,
                  const std::vector<std::string>& ancestors) {
        if (depth > 32)
            throw std::runtime_error("Recursive GUI include: " + file);
        pugi::xml_document doc;
        const auto bytes = assets.read(file);
        if (!doc.load_buffer(bytes.data(), bytes.size()))
            throw std::runtime_error("Invalid GUI XML: " + file);
        visit(doc.document_element(), directory(file), x, y, visible, depth + 1, ancestors);
    };
    visit = [&](pugi::xml_node parent, const std::string& base, float x, float y, bool visible, int depth,
                const std::vector<std::string>& ancestors) {
        for (auto node : parent.children()) {
            const std::string kind = node.name();
            if (kind == "include") {
                include(resolve(base, node.attribute("src").value()), x, y, visible, depth, ancestors);
                continue;
            }
            if (kind != "group" && kind != "image" && kind != "button" && kind != "checkbox" &&
                kind != "slider" && kind != "progress" && kind != "label" && kind != "editbox")
                continue;
            GuiElement element;
            element.kind = kind;
            element.id = node.attribute("id").value();
            element.baseline = kind == "label" || kind == "editbox";
            element.ancestors = ancestors;
            element.parentX = x;
            element.parentY = y;
            properties(element, node, base, 0);
            if (!element.normal.empty() && assets.contains(element.normal + ".animset")) {
                const auto animationPath = element.normal + ".animset";
                pugi::xml_document animation;
                const auto bytes = assets.read(animationPath);
                if (!animation.load_buffer(bytes.data(), bytes.size()))
                    throw std::runtime_error("Invalid GUI animation: " + animationPath);
                for (auto track : animation.child("animation_set").children("animation")) {
                    const std::string applier = track.attribute("applier").value();
                    if (applier == "picture" && track.child("const"))
                        element.normal =
                            resolve(directory(animationPath), track.child("const").text().as_string());
                    if (applier == "alpha" && track.child("const"))
                        element.alpha = track.child("const").text().as_float();
                    if (applier == "alpha")
                        for (auto key : track.child("keys").child("float").children("key"))
                            element.alphaKeys[key.attribute("relative_time").as_double()] =
                                key.text().as_float();
                }
            }
            element.x += x;
            element.y += y;
            if (auto found = visibility.find(element.id); found != visibility.end())
                element.visible = found->second;
            element.visible = visible && element.visible;
            if (kind != "group")
                result.push_back(element);
            auto children = ancestors;
            if (!element.id.empty())
                children.push_back(element.id);
            visit(node, base, element.x, element.y, element.visible, depth, children);
        }
    };
    include(path, 0, 0, true, 0, {});
    return result;
}
} // namespace deadtime
