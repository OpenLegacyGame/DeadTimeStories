#include "menu_scene.hpp"
#include <pugixml.hpp>
#include <stdexcept>
namespace deadtime {
MenuScene::MenuScene(const Assets& assets) {
    const std::string base = "menu/mainmenu";
    const auto bytes = assets.read(base + "/layout.scene");
    pugi::xml_document document;
    if (!document.load_buffer(bytes.data(), bytes.size()) || !document.child("scene"))
        throw std::runtime_error("Invalid menu scene");
    scene_.id = "menu";
    for (auto node : document.child("scene").children("object")) {
        auto object = std::make_unique<Object>();
        object->id = node.attribute("name").as_string();
        const auto position = node.child("position");
        object->x = position.attribute("x").as_float();
        object->y = position.attribute("y").as_float();
        object->z = position.attribute("z").as_float();
        object->texture = resolve(base, node.child("texture").attribute("src").as_string());
        object->enabled = node.child("enable").attribute("value").as_bool(true);
        for (auto particle : node.children("particles"))
            object->particleResources.push_back({resolve(base, particle.attribute("src").as_string()),
                                                 particle.attribute("presim").as_double()});
        for (auto model : node.children("scene")) {
            Object::SceneResource resource{resolve(base, model.attribute("src").as_string()),
                                           model.attribute("animation_looped").as_bool(true),
                                           {}};
            for (auto particle : model.children("particles"))
                resource.particles.push_back({resolve(base, particle.attribute("src").as_string()),
                                              particle.attribute("presim").as_double()});
            object->sceneResources.push_back(std::move(resource));
        }
        if (auto animation = node.child("animation"))
            object->scriptAnimation = Object::ScriptAnimation{
                AnimationSet(assets, resolve(base, animation.attribute("src").as_string()) + ".animset"), 0};
        scene_.objects.push_back(object.get());
        objects_.push_back(std::move(object));
    }
}
void MenuScene::hover(const std::string& button) {
    const auto selected = button == "continue" ? "start" : button;
    for (auto& object : objects_)
        if (object->id.ends_with("_hover"))
            object->enabled = object->id == selected + "_hover";
}
} // namespace deadtime
