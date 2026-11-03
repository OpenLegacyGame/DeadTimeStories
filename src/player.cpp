#include "player.hpp"
#include <SDL_image.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <pugixml.hpp>
#include <stdexcept>

namespace deadtime {
namespace {
std::string displayText(std::string text) {
    if (text.size() >= 2 && text.front() == '%' && text.back() == '%')
        text = text.substr(1, text.size() - 2);
    for (size_t pos = 0; (pos = text.find("\\n", pos)) != std::string::npos;)
        text.replace(pos, 2, "\n");
    // Dialogue separators contain voice timing in milliseconds, not visible text.
    std::string result;
    for (size_t i = 0; i < text.size();) {
        if (text[i] == '|') {
            ++i;
            while (i < text.size() && text[i] >= '0' && text[i] <= '9')
                ++i;
            if (i < text.size() && text[i] == '|')
                ++i;
            result += '\n';
        } else
            result += text[i++];
    }
    return result;
}
SDL_Surface* decode(const std::string& bytes) {
    auto* rw = SDL_RWFromConstMem(bytes.data(), static_cast<int>(bytes.size()));
    auto* raw = rw ? IMG_Load_RW(rw, 1) : nullptr;
    if (!raw)
        throw std::runtime_error(std::string("Image decode: ") + IMG_GetError());
    auto* rgba = SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(raw);
    if (!rgba)
        throw std::runtime_error(SDL_GetError());
    return rgba;
}
} // namespace
Player::Player(Assets& assets, Runtime& runtime, Profiles* profiles, GameSaves* saves, Settings* settings)
    : settings_(settings ? settings : &inspectionSettings_), saves_(saves), profiles_(profiles),
      assets_(assets), runtime_(runtime) {
    if (saves_ && !profiles_)
        throw std::runtime_error("Game saves require user identities");
    if (saves_ && profiles_->current())
        worldProfile_ = profiles_->ids().at(*profiles_->current());
}
Player::~Player() {
    SDL_StopTextInput();
    if (menuScene_)
        for (auto* object : menuScene_->scene().objects)
            runtime_.forgetVisual(*object);
    if (spiritLockSoundChannel_ >= 0 && runtime_.spiritLock())
        runtime_.spiritLock()->useAudioCallback(false);
    if (audio_)
        Mix_HaltChannel(-1);
    for (auto& [_, sound] : sounds_)
        Mix_FreeChunk(sound);
    if (audio_)
        Mix_CloseAudio();
    images_.clear();
    circlePatchImages_.clear();
    fonts_.clear();
    if (font_)
        TTF_CloseFont(font_);
    if (renderer_)
        SDL_DestroyRenderer(renderer_);
    if (window_) {
        SDL_ShowCursor(previousCursor_);
        SDL_DestroyWindow(window_);
    }
    TTF_Quit();
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}
Player::Image* Player::image(const std::string& name) {
    if (name.empty())
        return nullptr;
    const auto key = normalize(name);
    if (auto found = images_.find(key); found != images_.end())
        return found->second.get();
    try {
        const auto path = textureResource(assets_, key);
        if (path.empty())
            throw std::runtime_error("Missing texture: " + key);
        if (path.ends_with(".ifl")) {
            auto found = animations_.find(path);
            if (found == animations_.end())
                found =
                    animations_.emplace(path, Animation{ImageSequence(assets_, path), animationTime_}).first;
            return image(found->second.sequence.frame(animationTime_ - found->second.started));
        }
        auto* rgba = decode(assets_.read(path));
        std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface(rgba, SDL_FreeSurface);
        auto result = std::make_unique<Image>();
        result->width = rgba->w;
        result->height = rgba->h;
        result->mask.resize(static_cast<size_t>(rgba->w) * rgba->h, 255);
        auto maskPath = path.substr(0, path.find_last_of('.')) + ".a";
        if (assets_.contains(maskPath)) {
            std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> mask(decode(assets_.read(maskPath)),
                                                                          SDL_FreeSurface);
            if (mask->w != rgba->w || mask->h != rgba->h)
                throw std::runtime_error("Alpha dimensions differ: " + key);
            for (int y = 0; y < rgba->h; ++y)
                for (int x = 0; x < rgba->w; ++x) {
                    auto* dst = static_cast<unsigned char*>(rgba->pixels) + y * rgba->pitch + x * 4;
                    const auto* src =
                        static_cast<const unsigned char*>(mask->pixels) + y * mask->pitch + x * 4;
                    dst[3] = src[0];
                    result->mask[static_cast<size_t>(y) * rgba->w + x] = src[0];
                }
        }
        result->texture = SDL_CreateTextureFromSurface(renderer_, rgba);
        if (!result->texture)
            throw std::runtime_error(SDL_GetError());
        SDL_SetTextureBlendMode(result->texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(result->texture, SDL_ScaleModeLinear);
        auto* out = result.get();
        images_[key] = std::move(result);
        return out;
    } catch (const std::exception& e) {
        images_[key] = nullptr;
        runtime_.report(e.what());
        return nullptr;
    }
}
SDL_FRect Player::bounds(const Object& object, const Image& image) const {
    return {object.x, object.y, object.width > 0 ? object.width : float(image.width),
            object.height > 0 ? object.height : float(image.height)};
}
void Player::drawImage(const std::string& path, float x, float y, float w, float h, float alpha) {
    auto* img = image(path);
    if (!img)
        return;
    SDL_SetTextureAlphaMod(img->texture, static_cast<Uint8>(std::clamp(alpha, 0.f, 1.f) * 255));
    SDL_FRect rect{x, y, w > 0 ? w : float(img->width), h > 0 ? h : float(img->height)};
    SDL_RenderCopyF(renderer_, img->texture, nullptr, &rect);
}
void Player::drawScene(const Scene& scene) {
    auto sorted = scene.objects;
    std::stable_sort(sorted.begin(), sorted.end(), [](auto* a, auto* b) { return a->z > b->z; });
    for (auto* object : sorted)
        if (object->enabled && object->visible && object->alpha > 0) {
            drawImage(object->texture, object->x, object->y, object->width, object->height, object->alpha);
            drawModel(*object);
            drawParticles(*object);
        }
}
void Player::drawParticles(const Object& object) {
    drawParticleBatches(runtime_.particleBatches(object), object.alpha);
}
void Player::drawParticleBatches(const std::vector<ParticleBatch>& batches, float alpha, bool bottomOrigin) {
    for (const auto& batch : batches) {
        auto* texture = image(batch.layout->texture);
        if (!texture)
            continue;
        std::vector<SDL_Vertex> vertices;
        vertices.reserve(batch.sprites.size() * 6);
        for (const auto& sprite : batch.sprites) {
            if (sprite.alpha <= 0 || sprite.size == 0)
                continue;
            if (!std::isfinite(sprite.position.x) || !std::isfinite(sprite.position.y) ||
                !std::isfinite(sprite.size) || !std::isfinite(sprite.rotation)) {
                runtime_.report("Non-finite particle geometry");
                continue;
            }
            const float radians = sprite.rotation * 3.14159265358979323846f / 180, c = std::cos(radians),
                        s = std::sin(radians), half = sprite.size * .5f;
            const SDL_Color color{Uint8(sprite.color.x * 255), Uint8(sprite.color.y * 255),
                                  Uint8(sprite.color.z * 255), Uint8(sprite.alpha * alpha * 255)};
            std::array<SDL_Vertex, 4> quad;
            for (unsigned i = 0; i < 4; ++i) {
                const float x = (i == 0 || i == 1) ? -half : half, y = (i == 0 || i == 3) ? -half : half;
                auto point =
                    modelPoint(batch.transform, {sprite.position.x + x * c - y * s,
                                                 sprite.position.y + x * s + y * c, sprite.position.z});
                if (bottomOrigin)
                    point.y = 768 - point.y;
                const float u = float(sprite.tileX + (i >= 2 ? 1 : 0)) / batch.layout->slicesX,
                            v = float(sprite.tileY + ((i == 0 || i == 3) ? 1 : 0)) / batch.layout->slicesY;
                quad[i] = {{point.x, point.y}, color, {u, v}};
            }
            for (unsigned i : {0u, 1u, 2u, 0u, 2u, 3u})
                vertices.push_back(quad[i]);
        }
        SDL_SetTextureAlphaMod(texture->texture, 255);
        SDL_SetTextureBlendMode(texture->texture,
                                batch.layout->additive ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);
        if (!vertices.empty() && SDL_RenderGeometry(renderer_, texture->texture, vertices.data(),
                                                    int(vertices.size()), nullptr, 0) != 0)
            runtime_.report(std::string("Particle draw: ") + SDL_GetError());
        SDL_SetTextureBlendMode(texture->texture, SDL_BLENDMODE_BLEND);
    }
}
void Player::drawModel(const Object& object, bool bottomOrigin) {
    for (const auto& resource : object.sceneResources) {
        const auto path = normalize(resource.path);
        auto found = models_.find(path);
        if (found == models_.end()) {
            try {
                found = models_.emplace(path, std::make_unique<ColladaScene>(assets_, path)).first;
            } catch (const std::exception& e) {
                models_[path] = nullptr;
                runtime_.report(path + ": " + e.what());
                continue;
            }
        }
        if (!found->second)
            continue;
        const auto frame = found->second->frame(object.treeTime, object.treeLoopOverridden ? object.treeLooped
                                                                                           : resource.looped);
        for (const auto& mesh : frame.meshes) {
            auto* texture = image(mesh.texture);
            if (!texture)
                continue;
            std::vector<SDL_Vertex> vertices;
            vertices.reserve(mesh.vertices.size());
            const SDL_Color color{255, 255, 255, Uint8(std::clamp(object.alpha, 0.f, 1.f) * 255)};
            for (const auto& vertex : mesh.vertices)
                vertices.push_back(
                    {{object.x + vertex.position.x,
                      bottomOrigin ? 768 - object.y - vertex.position.y : object.y + vertex.position.y},
                     color,
                     {vertex.u, vertex.v}});
            SDL_SetTextureAlphaMod(texture->texture, 255);
            SDL_SetTextureBlendMode(texture->texture,
                                    mesh.additive ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);
            if (!vertices.empty() && SDL_RenderGeometry(renderer_, texture->texture, vertices.data(),
                                                        int(vertices.size()), nullptr, 0) != 0)
                runtime_.report(std::string("Model draw: ") + SDL_GetError());
            SDL_SetTextureBlendMode(texture->texture, SDL_BLENDMODE_BLEND);
        }
    }
}
void Player::drawHob(const HiddenObjectRound& round) {
    std::vector<const HobPiece*> pieces;
    for (const auto& piece : round.background)
        pieces.push_back(&piece);
    for (const auto& item : round.items)
        if (item.environment || item.target)
            for (const auto& piece : item.pieces)
                if (!piece.found)
                    pieces.push_back(&piece);
    std::stable_sort(pieces.begin(), pieces.end(), [](auto* a, auto* b) { return a->z > b->z; });
    const auto& feedback = runtime_.hobFeedback();
    const HobPiece* found = nullptr;
    if (feedback.found())
        found = &round.items[feedback.found()->first].pieces[feedback.found()->second];
    for (auto* p : pieces) {
        if (!p->visible || p->alpha <= 0)
            continue;
        if (p == found) {
            ActivityObject object;
            object.texture = p->texture;
            object.x = p->x;
            object.y = p->y;
            object.z = p->z;
            object.width = p->width;
            object.height = p->height;
            feedback.animate(object);
            drawActivity({object});
        } else
            drawImage(p->texture, p->x, p->y, p->width, p->height, p->alpha);
    }
    if (const auto* hint = runtime_.hobHint())
        drawParticles(*hint);
}
std::vector<std::string> Player::inventoryPage() const {
    std::vector<std::string> items;
    const auto& order = runtime_.inventoryOrder();
    for (auto it = order.rbegin(); it != order.rend(); ++it)
        if (*it != selection_ && (!inventoryFeedback_ || !inventoryFeedback_->flying(*it)))
            items.push_back(*it);
    const auto offset = std::min(size_t(std::max(0, inventoryOffset_)), items.size());
    const auto end = std::min(items.size(), offset + inventorySlots_.size());
    return {items.begin() + offset, items.begin() + end};
}
size_t Player::inventoryCount() const {
    return std::count_if(
        runtime_.inventoryOrder().begin(), runtime_.inventoryOrder().end(), [&](const auto& item) {
            return item != selection_ && (!inventoryFeedback_ || !inventoryFeedback_->flying(item));
        });
}
void Player::updateInventory() {
    if (auto* circle = runtime_.voodooCircle(); circle && circle->drawing() && !bookOpen())
        selection_ = "chalk";
    if (auto* worms = runtime_.leeches(); worms && worms->holding() && !bookOpen())
        selection_ = worms->heldOrange();
    if (!selection_.empty() && !runtime_.hasInventory(selection_))
        selection_.clear();
    const auto count = inventoryCount();
    inventoryOffset_ = std::clamp(inventoryOffset_, 0, std::max(0, int(count) - int(inventorySlots_.size())));
    int px = 0, py = 0;
    SDL_GetMouseState(&px, &py);
    float x = 0, y = 0;
    SDL_RenderWindowToLogical(renderer_, px, py, &x, &y);
    const bool overOpenZone =
        x >= inventoryOpenZone_.x && x < inventoryOpenZone_.x + inventoryOpenZone_.width &&
        y >= inventoryOpenZone_.y && y < inventoryOpenZone_.y + inventoryOpenZone_.height;
    const bool overInventory = x >= inventorySlots_.front().x &&
                               x < inventoryOpenZone_.x + inventoryOpenZone_.width &&
                               y >= inventoryBack_.at(1).y && y < 768;
    const bool allowed = !runtime_.locationHintsOpen() && !runtime_.carriage() &&
                         (!runtime_.skullsObey() || !runtime_.skullsObey()->busy()) && !runtime_.cutscene() &&
                         !runtime_.revealSpirits() && !bookOpen() && !runtime_.ouiji() &&
                         !runtime_.hexBones() &&
                         (!runtime_.voodooCircle() ||
                          (!runtime_.voodooCircle()->drawing() && !runtime_.voodooCircle()->busy()));
    if (!allowed) {
        if (inventoryAllowed_)
            inventoryPanel_->reset();
    } else {
        // Native Visible HUD construction restores the saved pin/open flag
        // directly, without replaying the grave animation after a book/menu.
        if (!inventoryAllowed_) {
            inventoryPanel_->reset(inventoryPinned_);
            inventoryOpen_ = inventoryPanel_->open();
        }
        inventoryPanel_->request(inventoryPinned_ || overOpenZone || (inventoryOpen_ && overInventory));
    }
    inventoryAllowed_ = allowed;
    syncInventoryPanel();
}
void Player::syncInventoryPanel() {
    if (!inventoryPanel_)
        return;
    inventoryOpen_ = inventoryPanel_->open();
    if (inventoryOverlayGeneration_ != inventoryPanel_->overlayGeneration()) {
        inventoryOverlayGeneration_ = inventoryPanel_->overlayGeneration();
        inventoryTransitionItems_ = inventoryPage();
    }
    for (const auto event : inventoryPanel_->takeSounds())
        sound(event == InventoryPanel::Sound::Open ? "sound/sfx/sfx_inv_on" : "sound/sfx/sfx_inv_off");
}
std::string Player::inventoryItemAt(float x, float y) {
    const auto items = inventoryPage();
    for (size_t slot = 0; slot < std::min(items.size(), inventoryOpen_ ? items.size() : size_t(1)); ++slot) {
        const auto found = inventoryImages_.find(items[slot]);
        const auto& place = inventorySlots_[slot];
        if (found != inventoryImages_.end() && hit(found->second, place.x, place.y, 0, 0, x, y))
            return items[slot];
    }
    return {};
}
bool Player::inventoryClick(float x, float y, bool activate) {
    for (const auto* hud : {&leftHud_, &rightHud_})
        for (const auto& element : *hud) {
            if (element.id != "button_pin_inventory" && element.id != "arrow_left" &&
                element.id != "arrow_right")
                continue;
            const int count = int(inventoryCount());
            if (element.id == "arrow_left" && (!inventoryOpen_ || inventoryOffset_ == 0))
                continue;
            if (element.id == "arrow_right" &&
                (!inventoryOpen_ || inventoryOffset_ + int(inventorySlots_.size()) >= count))
                continue;
            if (!hit(element.mask, element.x, element.y, element.width, element.height, x, y))
                continue;
            if (!activate)
                return true;
            if (element.id == "button_pin_inventory") {
                inventoryPinned_ = !inventoryPinned_;
                inventoryPanel_->request(inventoryPinned_);
                syncInventoryPanel();
                sound("sound/sfx/sfx_m_click_idle");
            } else if (inventoryOpen_)
                inventoryOffset_ += element.id == "arrow_left" ? -1 : 1;
            updateInventory();
            return true;
        }
    if (const auto item = inventoryItemAt(x, y); !item.empty()) {
        if (!activate)
            return true;
        selection_ = item;
        if (!runtime_.leeches() || !runtime_.leeches()->holding())
            sound("sound/sfx/sfx_inv_item_pick");
        return true;
    }
    if (x >= inventoryOpenZone_.x && x < inventoryOpenZone_.x + inventoryOpenZone_.width &&
        y >= inventoryOpenZone_.y && y < inventoryOpenZone_.y + inventoryOpenZone_.height)
        return true;
    if (inventoryOpen_) {
        // Transparent gaps in the graves still belong to the inventory panel.
        // Letting them fall through can activate an exit behind the HUD.
        if (x >= inventorySlots_.front().x && x < inventoryOpenZone_.x + inventoryOpenZone_.width &&
            y >= inventoryBack_.at(1).y && y < 768)
            return true;
        for (const auto& element : inventoryBack_)
            if (element.visible &&
                hit(element.normal, element.x, element.y, element.width, element.height, x, y))
                return true;
    }
    return false;
}
bool Player::prepareBook() {
    if (runtime_.hobFeedback().transient())
        return false;
    if (menuView_ != MenuView::None || !dialogs_.empty() || runtime_.cutscene() || runtime_.carriage() ||
        runtime_.locationHintsOpen() || (runtime_.ouiji() && runtime_.ouiji()->busy()))
        return false;
    if (auto* skulls = runtime_.skullsObey(); skulls && skulls->busy())
        return false;
    if (auto* circle = runtime_.voodooCircle(); circle && circle->busy())
        return false;
    cancelPointerInput();
    if (inventoryFeedback_)
        inventoryFeedback_->clear();
    if (inventoryPanel_) {
        inventoryPanel_->reset();
        inventoryAllowed_ = false;
        syncInventoryPanel();
    }
    clearTutorial();
    if (runtime_.revealSpirits())
        runtime_.closeRevealSpirits();
    selection_.clear();
    pressedInventory_.clear();
    draggingInventory_ = false;
    return true;
}
void Player::toggleSpellBook() {
    if (!runtime_.flag("enableSpellBook") || !prepareBook())
        return;
    if (!spellBook_) {
        spellBook_ = std::make_unique<SpellBook>(assets_);
        spellBookControls_ = loadGui(assets_, "spellbook/controls/layout.gui");
    }
    spellBook_->refresh([&](const auto& name) { return runtime_.flag("enableSpell:" + name); });
    spellBookOpen_ = !spellBookOpen_;
    plantBookOpen_ = false;
    plantBookPressed_.clear();
    selection_.clear();
    pressedInventory_.clear();
    draggingInventory_ = false;
    if (spellBookOpen_) {
        spellBook_->open(runtime_.latestSpell());
        sound("sound/sfx/sfx_book_open");
        runtime_.requestTutorial(TutorialId::SpellBook);
    } else {
        sound("sound/sfx/sfx_book_close");
    }
}
void Player::togglePlantBook() {
    if (plantBookOpen_) {
        plantBookAction("close");
        return;
    }
    if (!runtime_.flag("enablePlantBook") || !prepareBook())
        return;
    if (!plantBook_) {
        plantBook_ = std::make_unique<PlantBook>(assets_);
        plantBookControls_ = loadGui(assets_, "plantbook/controls/layout.gui");
    }
    plantBook_->refresh([&](const auto& name) { return runtime_.flag("enablePlantBookPage:" + name); });
    if (!plantBook_->pageCount())
        return;
    plantBookOpen_ = true;
    spellBookOpen_ = false;
    plantBookPressed_.clear();
    sound("sound/sfx/sfx_book_open");
}
std::vector<GuiElement> Player::plantBookControls() {
    auto controls = plantBookControls_;
    for (auto& element : controls) {
        if (element.id == "prev_page")
            element.enabled = plantBook_->hasPrevious();
        if (element.id == "next_page")
            element.enabled = plantBook_->hasNext();
        if (element.id == "arrow_left")
            element.visible = plantBook_->hasPrevious();
        if (element.id == "arrow_right")
            element.visible = plantBook_->hasNext();
    }
    return controls;
}
std::string Player::plantBookButtonAt(float x, float y) {
    const auto controls = plantBookControls();
    for (auto it = controls.rbegin(); it != controls.rend(); ++it)
        if (it->kind == "button" && it->visible && it->enabled)
            if (hit(it->mask.empty() ? it->normal : it->mask, it->x, it->y, it->width, it->height, x, y))
                return it->id;
    return {};
}
void Player::plantBookAction(const std::string& action) {
    if (action == "close") {
        plantBookOpen_ = false;
        plantBookPressed_.clear();
        cancelPointerInput();
        sound("sound/sfx/sfx_book_close");
    } else if ((action == "prev_page" || action == "next_page") &&
               plantBook_->turn(action == "prev_page" ? -1 : 1))
        sound("sound/sfx/sfx_book_open");
}
bool Player::plantBookInput(const SDL_Event& event) {
    if (!plantBookOpen_ || hasDialog())
        return false;
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        plantBookPressed_.clear();
    if (event.type == SDL_KEYDOWN && !event.key.repeat) {
        const auto key = event.key.keysym.sym;
        if (key == SDLK_ESCAPE || key == SDLK_p)
            plantBookAction("close");
        else if (key == SDLK_s)
            toggleSpellBook();
        else if (key == SDLK_LEFT)
            plantBookAction("prev_page");
        else if (key == SDLK_RIGHT)
            plantBookAction("next_page");
    }
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
        plantBookPressed_ = plantBookButtonAt(float(event.button.x), float(event.button.y));
    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        const auto action = plantBookButtonAt(float(event.button.x), float(event.button.y));
        const bool activate = !action.empty() && action == plantBookPressed_;
        plantBookPressed_.clear();
        if (activate)
            plantBookAction(action);
    }
    return true;
}
void Player::drawPlantBook() {
    if (!plantBookOpen_)
        return;
    const auto& book = *plantBook_;
    drawImage("common/half_dark", 0, 0, 1024, 768);
    drawImage(book.background, book.x, book.y);
    for (bool right : {false, true})
        if (const auto* page = right ? book.right() : book.left()) {
            auto elements = loadGui(assets_, page->layout);
            for (auto& element : elements) {
                element.x += book.x + (right ? book.rightX : book.leftX);
                element.y += book.y + (right ? book.rightY : book.leftY);
            }
            drawGui(elements);
        }
    int px = 0, py = 0;
    SDL_GetMouseState(&px, &py);
    float x = 0, y = 0;
    SDL_RenderWindowToLogical(renderer_, px, py, &x, &y);
    const auto hovered = plantBookButtonAt(x, y);
    auto controls = plantBookControls();
    for (const auto& source : controls)
        if (source.id == hovered && source.enabled)
            for (auto& target : controls)
                if (auto texture = source.hoverTextures.find(target.id);
                    texture != source.hoverTextures.end())
                    target.normal = texture->second;
    drawGui(controls);
}
bool Player::spellBookButtonEnabled(const GuiElement& control) const {
    if (control.id == "prev_page")
        return spellBook_->hasPrevious();
    if (control.id == "next_page")
        return spellBook_->hasNext();
    if (control.id == "recipes")
        return !spellBook_->recipes();
    if (control.id == "instructions")
        return spellBook_->recipes();
    return control.enabled;
}
const GuiElement* Player::spellBookButtonAt(float x, float y) {
    for (auto it = spellBookControls_.rbegin(); it != spellBookControls_.rend(); ++it) {
        if (it->kind != "button" || !it->visible)
            continue;
        if ((it->id == "prev_page" || it->id == "next_page") && !spellBookButtonEnabled(*it))
            continue;
        const auto& mask = it->mask.empty() ? it->normal : it->mask;
        if (hit(mask, it->x, it->y, it->width, it->height, x, y))
            return &*it;
    }
    return nullptr;
}
void Player::spellBookAction(const std::string& action) {
    spellBookPressed_ = nullptr;
    if (action == "close") {
        spellBookOpen_ = false;
        clearTutorial();
        sound("sound/sfx/sfx_book_close");
    } else if (action == "recipes" || action == "instructions") {
        const bool recipes = action == "recipes";
        if (spellBook_->recipes() != recipes) {
            spellBook_->selectTab(recipes, runtime_.latestSpell());
            sound("sound/sfx/sfx_book_open");
        }
    } else if (action == "next_page" || action == "prev_page") {
        if (spellBook_->turn(action == "next_page" ? 1 : -1))
            sound("sound/sfx/sfx_book_open");
    }
}
void Player::drawSpellBook() {
    if (!spellBookOpen_)
        return;
    auto& book = *spellBook_;
    book.refresh([&](const auto& name) { return runtime_.flag("enableSpell:" + name); });
    drawImage("common/half_dark", 0, 0, 1024, 768);
    drawImage(book.background, book.x, book.y);
    const auto& page = book.page();
    auto elements = loadGui(assets_, page.layout, {{"enabled", page.enabled}, {"disabled", !page.enabled}});
    for (auto& element : elements) {
        element.x += book.x + book.pageX;
        element.y += book.y + book.pageY;
    }
    drawGui(elements);
    if (page.recipe && page.enabled) {
        const std::string ingredientFont = "fonts/spell_book_ingredient";
        const int ascent = TTF_FontAscent(font(ingredientFont));
        for (const auto& ingredient : page.ingredients) {
            const auto x = book.x + book.pageX + ingredient.x, y = book.y + book.pageY + ingredient.y;
            // The source SceneText and its checkmark child share a baseline.
            drawText(ingredient.text, int(x), int(y) - ascent, 0, {0, 0, 0, 255}, ingredientFont);
            if (runtime_.wasFound(ingredient.item))
                drawImage(book.checkMark, x + book.checkX, y + book.checkY);
        }
    }
    int mouseX = 0, mouseY = 0;
    SDL_GetMouseState(&mouseX, &mouseY);
    float x = 0, y = 0;
    SDL_RenderWindowToLogical(renderer_, mouseX, mouseY, &x, &y);
    const auto* hovered = spellBookButtonAt(x, y);
    auto controls = spellBookControls_;
    for (auto& element : controls) {
        if (element.kind == "button")
            element.enabled = spellBookButtonEnabled(element);
        if (hovered) {
            const auto texture = hovered->hoverTextures.find(element.id);
            if (texture != hovered->hoverTextures.end())
                element.normal = texture->second;
        }
        if (element.id == "arrow_left" || element.id == "arrow_right") {
            element.visible = element.id == "arrow_left" ? book.hasPrevious() : book.hasNext();
            element.id = "book_" + element.id;
        }
    }
    drawGui(controls);
}
void Player::drawInventory() {
    auto left = leftHud_, right = rightHud_;
    const int count = int(inventoryCount());
    for (auto* hud : {&left, &right})
        for (auto& element : *hud) {
            if (element.id == "inventory_image")
                element.visible = !inventoryOpen_;
            if (element.id == "arrow_left" || element.id == "arrow_right") {
                element.visible = inventoryOpen_ && count > int(inventorySlots_.size());
                element.enabled = element.id == "arrow_left"
                                      ? inventoryOffset_ > 0
                                      : count > inventoryOffset_ + int(inventorySlots_.size());
            }
            if (element.id == "button_pin_inventory" && inventoryPinned_) {
                element.normal = element.normalChecked;
                element.hover = element.hoverChecked;
                element.clicked = element.clickedChecked;
            }
        }
    drawGui(left);
    if (inventoryOpen_)
        drawGui(inventoryBack_);
    drawGui(right);
    const auto items = inventoryPage();
    // Native first slot remains visible on the collapsed first gravestone.
    for (size_t slot = 0; slot < std::min(items.size(), inventoryOpen_ ? items.size() : size_t(1)); ++slot) {
        const auto found = inventoryImages_.find(items[slot]);
        if (found != inventoryImages_.end())
            drawImage(found->second, inventorySlots_[slot].x, inventorySlots_[slot].y);
    }
    if (inventoryPanel_) {
        // These two native HUD scenes use the bottom-origin orthographic
        // camera, unlike the normal 2D GUI and location scenes.
        if (const auto* model = inventoryPanel_->model())
            drawModel(*model, true);
        drawParticleBatches(inventoryPanel_->particles(), 1, true);
        const auto alpha = inventoryPanel_->overlayAlpha();
        if (alpha > 0)
            drawImage("hud/common/web_graves", inventoryBack_.at(1).x, inventoryBack_.at(1).y, 0, 0, alpha);
        drawImage("hud/common/back_candles_graves", inventoryBack_.at(1).x, inventoryBack_.at(1).y);
        if (alpha > 0)
            for (size_t slot = 1; slot < inventoryTransitionItems_.size(); ++slot) {
                const auto icon = inventoryImages_.find(inventoryTransitionItems_[slot]);
                if (icon != inventoryImages_.end())
                    drawImage(icon->second, inventorySlots_[slot].x, inventorySlots_[slot].y, 0, 0, alpha);
            }
    }
    if (inventoryFeedback_)
        for (const auto& flight : inventoryFeedback_->flights()) {
            if (!flight->arrived)
                if (const auto found = inventoryImages_.find(flight->item); found != inventoryImages_.end())
                    drawImage(found->second, flight->x, flight->y);
            drawParticleBatches(inventoryFeedback_->particles(*flight));
            if (flight->arrived && !flight->feedbackFinished && inventoryTooltips_.contains(flight->item)) {
                const auto slot = std::find(items.begin(), items.end(), flight->item);
                if (slot != items.end()) {
                    const auto& place = inventorySlots_[size_t(slot - items.begin())];
                    const auto* icon = image(inventoryImages_.at(flight->item));
                    if (icon)
                        drawTutorial(inventoryTooltips_.at(flight->item), place.x + icon->width * .5f,
                                     place.y + 20, InventoryFeedback::labelAlpha(flight->labelElapsed));
                }
            }
        }
    int px = 0, py = 0;
    SDL_GetMouseState(&px, &py);
    float x = 0, y = 0;
    SDL_RenderWindowToLogical(renderer_, px, py, &x, &y);
    if (selection_.empty() && !hasDialog()) {
        const auto item = inventoryItemAt(x, y);
        const bool automatic =
            inventoryFeedback_ &&
            std::any_of(inventoryFeedback_->flights().begin(), inventoryFeedback_->flights().end(),
                        [&](const auto& f) { return f->item == item && f->arrived && !f->feedbackFinished; });
        if (!automatic && !item.empty() && inventoryTooltips_.contains(item))
            drawTutorial(inventoryTooltips_.at(item), x, inventorySlots_.front().y);
    }
}
void Player::drawMixing(SpellMixing& mix) {
    auto objects = mix.objects();
    if (!mixHeld_.empty()) {
        const auto& ingredient = mix.ingredients().at(mixHeld_);
        for (auto& object : objects)
            if (object.id == ingredient.object ||
                (ingredient.fromTable && object.id == ingredient.inventory + "_big"))
                object.visible = false;
    }
    if (mixMortarHeld_)
        for (auto& object : objects) {
            for (const auto* role :
                 {"mortar", "mortar_with_ingredients", "mortar_full", "mortar_mixture_ready"})
                if (mix.sceneObjects().contains(role) && object.id == mix.sceneObjects().at(role))
                    object.visible = false;
        }
    drawActivity(objects);
    if (mix.readyToStir()) {
        const bool clockwise = mix.direction() == "cw";
        const auto& arrow = mix.object(mix.sceneObjects().at(clockwise ? "hint_cw" : "hint_ccw"));
        const auto color = mix.hintColor;
        drawText(std::to_string(mix.remainingGestures()),
                 int(arrow.x + (clockwise ? mix.hintClockwiseX : mix.hintCounterclockwiseX)),
                 int(arrow.y + (clockwise ? mix.hintClockwiseY : mix.hintCounterclockwiseY)), 0,
                 {Uint8(color >> 16), Uint8(color >> 8), Uint8(color), Uint8(color >> 24)}, mix.hintFont);
    }
}
unsigned Player::mixAngle(float x, float y) const {
    const auto* mix = runtime_.mixing();
    const double dx = x - mixGrabX_ - std::trunc((mix->mortarLeft + mix->mortarRight) / 2);
    const double dy = y - mixGrabY_ - std::trunc((mix->mortarTop + mix->mortarBottom) / 2);
    const double radius = std::hypot(dx, dy);
    if (radius == 0)
        return 0;
    int angle = int(std::acos(std::clamp(-dy / radius, -1., 1.)) * 180 / 3.14159265358979323846);
    if (dx <= 0)
        angle = -angle;
    if (angle < 0)
        angle += 360;
    return unsigned(angle) % 360;
}
void Player::moveMixPestle(float x, float y) {
    auto* mix = runtime_.mixing();
    if (!mix || !mix->stirring())
        return;
    const float cx = std::trunc((mix->mortarLeft + mix->mortarRight) / 2),
                cy = std::trunc((mix->mortarTop + mix->mortarBottom) / 2);
    const float rx = (mix->mortarRight - mix->mortarLeft) / 2, ry = (mix->mortarBottom - mix->mortarTop) / 2;
    float dx = x - mixGrabX_ - cx, dy = y - mixGrabY_ - cy;
    while ((dx * dx) / (rx * rx) + (dy * dy) / (ry * ry) > 1) {
        dx *= 0.95f;
        dy *= 0.95f;
    }
    auto& pestle = mix->object(mix->sceneObjects().at("pestle_in_mortar"));
    pestle.x = std::trunc(cx + dx);
    pestle.y = std::trunc(cy + dy);
    mix->moveStir(mixAngle(pestle.x + mixGrabX_, pestle.y + mixGrabY_));
}
void Player::clickMixing(float x, float y) {
    auto& mix = *runtime_.mixing();
    if (inventoryClick(x, y)) {
        mixHeld_.clear();
        mixMortarHeld_ = false;
        return;
    }
    auto objectHit = [&](const std::string& id) {
        const auto& object = mix.object(id);
        return object.enabled && object.visible &&
               hit(object.texture, object.x, object.y, object.width, object.height, x, y);
    };
    const auto held = !mixHeld_.empty() ? mixHeld_ : selection_;
    if (!held.empty()) {
        if (mix.bottleFilled()) {
            if (objectHit(mix.sceneObjects().at("full_bottle")) && runtime_.finishMixing(held)) {
                mixHeld_.clear();
                selection_.clear();
                mixMortarHeld_ = false;
                hint_.clear();
                sound("sound/sfx/mg/mg_finish");
            }
        } else if (!mix.arranged()) {
            bool overLoa = objectHit(mix.sceneObjects().at("loa_mask")) || objectHit(mix.bottleSlot);
            for (const auto& slot : mix.slots())
                overLoa = overLoa || objectHit(slot);
            if (overLoa && runtime_.placeMixing(held)) {
                mixHeld_.clear();
                selection_.clear();
                sound("sound/sfx/mg/spellmixing/mg_item_on_table");
            }
        } else if (!mixHeld_.empty()) {
            bool overMortar = false;
            for (const auto* role : {"mortar", "mortar_with_ingredients", "mortar_full"}) {
                const auto found = mix.sceneObjects().find(role);
                if (found != mix.sceneObjects().end())
                    overMortar = overMortar || objectHit(found->second);
            }
            if (overMortar) {
                if (runtime_.mixIngredient(held))
                    sound("sound/sfx/mg/spellmixing/mg_item_mix");
                else
                    sound("sound/sfx/mg/mg_wrong_name");
                mixHeld_.clear();
            }
        }
        return;
    }
    if (mixMortarHeld_) {
        for (const auto& [_, ingredient] : mix.ingredients())
            if (ingredient.bottle && objectHit(ingredient.object) && mix.fillBottle()) {
                mixMortarHeld_ = false;
                sound("sound/sfx/mg/spellmixing/mg_pour");
                return;
            }
        return;
    }
    if (mix.mixtureReady() && !mix.bottleFilled() &&
        objectHit(mix.sceneObjects().at("mortar_mixture_ready"))) {
        mixMortarHeld_ = true;
        sound("sound/sfx/sfx_inv_item_pick");
        return;
    }
    if (mix.bottleFilled() && objectHit(mix.sceneObjects().at("full_bottle"))) {
        if (mix.finalInventory().empty() && runtime_.finishMixing()) {
            hint_.clear();
            sound("sound/sfx/mg/mg_finish");
        } else
            showHint(runtime_.string("spellmixing/banish_spirits/hints/red_string_missing"));
        return;
    }
    for (const auto& [id, ingredient] : mix.ingredients()) {
        if (ingredient.bottle)
            continue;
        if (mix.arranged() && mix.slot(id) && !mix.inMortar(id) && objectHit(ingredient.object)) {
            mixHeld_ = id;
            return;
        }
        if (!mix.arranged() && ingredient.fromTable && !mix.slot(id)) {
            for (const auto& source : mix.objects())
                if (source.id == id + "_big" && source.visible &&
                    hit(source.texture, source.x, source.y, source.width, source.height, x, y)) {
                    mixHeld_ = id;
                    return;
                }
        }
    }
    for (const auto& object : mix.objects())
        if (object.id == "exit" && object.contains(x, y)) {
            runtime_.closeMixing();
            mixHeld_.clear();
            mixMortarHeld_ = false;
            return;
        }
}
void Player::drawOuiji(OuijiBoard& board) {
    auto& pointer = board.planchette();
    if (auto* img = image(pointer.texture))
        board.setPlanchetteSize(float(img->width), float(img->height));
    drawActivity(board.objects());
}
void Player::drawVoodoo(VoodooDoll& doll) {
    auto objects = doll.objects();
    if (const auto index = doll.ingredient(voodooHeld_))
        for (auto& object : objects)
            if (object.id == doll.ingredients()[*index].object)
                object.visible = false;
    drawActivity(objects);
}
void Player::clickVoodoo(float x, float y) {
    auto& doll = *runtime_.voodoo();
    voodooHint_.clear();
    doll.feedback.clear();
    if (doll.complete())
        return;
    if (inventoryClick(x, y)) {
        voodooHeld_.clear();
        return;
    }
    const auto& central = doll.object("slot_central");
    const bool centralTarget =
        hit(central.texture, central.x, central.y, central.width, central.height, x, y);
    const auto held = voodooHeld_.empty() ? selection_ : voodooHeld_;
    if (!held.empty()) {
        if (doll.object("exit").contains(x, y)) {
            voodooHeld_.clear();
            selection_.clear();
            return;
        }
        if (!doll.arranged()) {
            const auto& back = doll.object("back");
            if (!centralTarget && !hit(back.texture, back.x, back.y, back.width, back.height, x, y))
                return;
            if (runtime_.placeVoodoo(held, centralTarget, x, y))
                selection_.clear();
        } else if (centralTarget && !voodooHeld_.empty()) {
            runtime_.applyVoodoo(held);
        } else if (voodooHeld_.empty()) {
            doll.feedback = "spellmixing/common/hints/wrong_ingredient";
        }
        if (doll.arranged()) {
            voodooHeld_.clear();
            selection_.clear();
        }
        if (!doll.feedback.empty()) {
            voodooHint_ = runtime_.string(doll.feedback);
            voodooHintX_ = x;
            voodooHintY_ = y;
        }
        return;
    }
    std::vector<size_t> candidates;
    if (doll.arranged())
        for (size_t i = doll.ingredients().size(); i-- > 0;)
            candidates.push_back(i);
    std::stable_sort(candidates.begin(), candidates.end(), [&](auto left, auto right) {
        return doll.object(doll.ingredients()[left].object).z <
               doll.object(doll.ingredients()[right].object).z;
    });
    for (auto i : candidates) {
        const auto& item = doll.ingredients()[i];
        const auto& sprite = doll.object(item.object);
        if (!sprite.visible || !sprite.interactive)
            continue;
        if (hit(sprite.texture, sprite.x - sprite.pivotX, sprite.y - sprite.pivotY, sprite.width,
                sprite.height, x, y)) {
            voodooHeld_ = item.inventory;
            sound("sound/sfx/sfx_inv_item_pick");
            return;
        }
    }
    if (doll.object("exit").contains(x, y))
        runtime_.closeVoodoo();
}
void Player::drawActivity(const std::vector<ActivityObject>& objects) {
    std::vector<const ActivityObject*> sorted;
    for (const auto& object : objects)
        sorted.push_back(&object);
    std::stable_sort(sorted.begin(), sorted.end(), [](auto* a, auto* b) { return a->z > b->z; });
    for (auto* object : sorted) {
        if (!object->enabled || !object->visible || object->alpha <= 0 || object->scale <= 0 ||
            object->texture.empty())
            continue;
        auto* img = image(object->texture);
        if (!img)
            continue;
        SDL_FPoint origin{object->pivotX * object->scale, object->pivotY * object->scale};
        SDL_FRect destination{object->x - origin.x, object->y - origin.y,
                              (object->width > 0 ? object->width : float(img->width)) * object->scale,
                              (object->height > 0 ? object->height : float(img->height)) * object->scale};
        SDL_SetTextureAlphaMod(img->texture, Uint8(std::clamp(object->alpha, 0.f, 1.f) * 255));
        SDL_SetTextureBlendMode(img->texture, object->additive ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);
        if (object->repeatX) {
            for (float at = 0; at < object->width; at += float(img->width)) {
                const float width = std::min(float(img->width), object->width - at);
                SDL_Rect source{0, 0, int(std::ceil(width)), img->height};
                SDL_FRect tile{destination.x + at * object->scale, destination.y, width * object->scale,
                               destination.h};
                SDL_FPoint pivot{origin.x - at * object->scale, origin.y};
                SDL_RenderCopyExF(renderer_, img->texture, &source, &tile, object->angle, &pivot,
                                  SDL_FLIP_NONE);
            }
        } else
            SDL_RenderCopyExF(renderer_, img->texture, nullptr, &destination, object->angle, &origin,
                              SDL_FLIP_NONE);
        SDL_SetTextureBlendMode(img->texture, SDL_BLENDMODE_BLEND);
    }
}
void Player::drawSpiritLock(SpiritLock& lock) {
    std::vector<const ActivityObject*> sorted;
    for (const auto& object : lock.objects())
        sorted.push_back(&object);
    std::stable_sort(sorted.begin(), sorted.end(), [](auto* a, auto* b) { return a->z > b->z; });
    for (const auto* object : sorted) {
        drawActivity({*object});
        if (const auto* effect = lock.effect(object->id); effect && effect->enabled && effect->visible) {
            drawModel(*effect);
            drawParticles(*effect);
        }
    }
}
const ActivityObject* Player::spiritLockTarget(float x, float y) {
    auto* lock = runtime_.spiritLock();
    if (!lock)
        return nullptr;
    std::vector<const ActivityObject*> sorted;
    for (auto it = lock->objects().rbegin(); it != lock->objects().rend(); ++it)
        if (it->interactive &&
            (it->id == "exit" || it->id == "lock" || it->id == "all_pieces" || it->id.starts_with("piece_")))
            sorted.push_back(&*it);
    std::stable_sort(sorted.begin(), sorted.end(), [](auto* a, auto* b) { return a->z < b->z; });
    for (const auto* object : sorted)
        if (hitActivity(*object, x, y))
            return object;
    return nullptr;
}
void Player::clickSpiritLock(float x, float y) {
    auto& lock = *runtime_.spiritLock();
    if (lock.busy())
        return;
    hintBubble_.clear();
    if (inventoryClick(x, y))
        return;
    const auto* object = spiritLockTarget(x, y);
    if (!selection_.empty()) {
        if (object && object->id.starts_with("piece_")) {
            const auto slot = size_t(std::stoul(object->id.substr(6))) - 1;
            if (runtime_.applySpiritLockPiece(slot, selection_))
                selection_.clear();
        } else
            lock.wrongItem();
        return;
    }
    if (object && object->id == "exit") {
        runtime_.closeSpiritLock();
        return;
    }
    lock.idleClick();
}
void Player::drawApothecary(Apothecary& shelves) {
    auto objects = shelves.objects();
    for (auto& object : objects)
        if (const auto* effect = shelves.effect(object.id))
            object.z = effect->z;
    std::stable_sort(objects.begin(), objects.end(), [](const auto& a, const auto& b) { return a.z > b.z; });
    for (const auto& object : objects) {
        drawActivity({object});
        if (const auto* effect = shelves.effect(object.id); effect && effect->enabled && effect->visible)
            drawParticles(*effect);
        if (object.id == "sheet_zoomed" && object.visible) {
            GuiElement text;
            text.kind = "label";
            text.font = shelves.riddleFont();
            text.color = shelves.riddleColor();
            text.text = runtime_.string("puzzles/sort_apothecary_items/riddle");
            text.horizontalAlign = "left";
            text.verticalAlign = "top";
            text.x = object.x + 55;
            text.y = object.y + 100;
            text.width = object.width - 200;
            text.height = object.height - 100;
            auto* face = font(text.font);
            const int lineSkip = TTF_FontLineSkip(face);
            TTF_SetFontLineSkip(face, std::max(1, int(std::lround(lineSkip * .7))));
            drawGui({text});
            TTF_SetFontLineSkip(face, lineSkip);
        }
    }
}
const ActivityObject* Player::apothecaryTarget(float x, float y) {
    auto* shelves = runtime_.apothecary();
    if (!shelves || shelves->sheetOpen())
        return nullptr;
    std::vector<const ActivityObject*> sorted;
    for (auto it = shelves->objects().rbegin(); it != shelves->objects().rend(); ++it)
        if (it->interactive && (it->id == "exit" || it->id == "sheet" || it->id.starts_with("item_") ||
                                (it->id.starts_with("shelf_") && !it->id.ends_with("_item_pos"))))
            sorted.push_back(&*it);
    std::stable_sort(sorted.begin(), sorted.end(), [](auto* a, auto* b) { return a->z < b->z; });
    for (auto* object : sorted)
        if (hitActivity(*object, x, y))
            return object;
    return nullptr;
}
void Player::clickApothecary(float x, float y) {
    auto& shelves = *runtime_.apothecary();
    if (shelves.busy())
        return;
    if (shelves.sheetOpen()) {
        shelves.toggleSheet();
        return;
    }
    hintBubble_.clear();
    if (inventoryClick(x, y)) {
        shelves.cancel();
        return;
    }
    const auto* target = apothecaryTarget(x, y);
    if (target && target->id == "sheet") {
        shelves.toggleSheet();
        return;
    }
    if (target && target->id == "exit") {
        runtime_.closeApothecary();
        return;
    }
    std::optional<size_t> slot;
    if (target && target->id.starts_with("shelf_"))
        slot = size_t(std::stoul(target->id.substr(6))) - 1;
    if (target && target->id.starts_with("item_")) {
        const size_t index = size_t(std::stoul(target->id.substr(5))) - 1;
        const auto found =
            std::find(shelves.slots().begin(), shelves.slots().end(), std::optional<size_t>(index));
        if (found != shelves.slots().end())
            slot = size_t(found - shelves.slots().begin());
    }
    if (!selection_.empty()) {
        if (slot)
            runtime_.placeApothecaryItem(*slot, selection_);
        selection_.clear();
        shelves.cancel();
        return;
    }
    if (slot)
        shelves.select(*slot);
    else
        shelves.idleClick();
}
std::optional<size_t> Player::sailPieceAt(float x, float y) {
    auto* cloth = runtime_.sail();
    if (!cloth)
        return {};
    std::vector<size_t> sorted;
    for (size_t i = 0; i < cloth->pieces().size(); ++i)
        sorted.push_back(i);
    std::stable_sort(sorted.begin(), sorted.end(),
                     [&](auto a, auto b) { return cloth->piece(a).z < cloth->piece(b).z; });
    for (auto index : sorted)
        if (hitActivity(cloth->piece(index), x, y))
            return index;
    return {};
}
void Player::clickSail(float x, float y) {
    auto& cloth = *runtime_.sail();
    if (cloth.busy() || cloth.gripping())
        return;
    hintBubble_.clear();
    if (inventoryClick(x, y))
        return;
    if (!selection_.empty()) {
        runtime_.applySailInventory(selection_, x, y);
        selection_.clear();
        return;
    }
    if (y > cloth.exitY())
        runtime_.closeSail();
}
const ActivityObject* Player::leechesTarget(float x, float y) {
    auto* worms = runtime_.leeches();
    if (!worms)
        return nullptr;
    std::vector<const ActivityObject*> sorted;
    for (auto it = worms->objects().rbegin(); it != worms->objects().rend(); ++it)
        if (it->interactive)
            sorted.push_back(&*it);
    std::stable_sort(sorted.begin(), sorted.end(), [](auto* a, auto* b) { return a->z < b->z; });
    for (auto* object : sorted)
        if (hitActivity(*object, x, y))
            return object;
    return nullptr;
}
void Player::clickLeeches(float x, float y) {
    auto& worms = *runtime_.leeches();
    if (worms.complete())
        return;
    const auto previous = selection_;
    if (inventoryClick(x, y)) {
        if (worms.holding()) {
            selection_ = previous;
            worms.refuseCancel();
        }
        return;
    }
    const auto* target = leechesTarget(x, y);
    const auto id = target ? target->id : "";
    if (worms.holding()) {
        runtime_.inspectLeeches(id);
        if (!worms.holding())
            selection_.clear();
        return;
    }
    if (id == "exit") {
        runtime_.closeLeeches();
        return;
    }
    if (id.starts_with("leech_")) {
        const size_t index = size_t(std::stoul(id.substr(6))) - 1;
        if (!runtime_.catchLeech(index, selection_) && !Leeches::isOrange(selection_))
            selection_.clear();
        return;
    }
    if (!selection_.empty()) {
        if (id == "bucket" && Leeches::isOrange(selection_)) {
            sound("sound/sfx/sfx_m_click_idle");
            return;
        }
        selection_.clear();
        sound("sound/sfx/sfx_inv_item_put");
        runtime_.inspectLeeches("");
        return;
    }
    runtime_.inspectLeeches(id);
}
void Player::drawSoftwareCirclePatch(const CircleShape& shape, const Image& source) {
    auto& cache = circlePatchImages_[shape.sprite.texture];
    if (!cache.image) {
        std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> pixels(
            decode(assets_.read(textureResource(assets_, shape.sprite.texture))), SDL_FreeSurface);
        auto image = std::make_unique<Image>();
        image->width = source.width;
        image->height = source.height;
        image->texture = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
                                           source.width, source.height);
        if (!image->texture)
            throw std::runtime_error(SDL_GetError());
        SDL_SetTextureBlendMode(image->texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(image->texture, SDL_ScaleModeLinear);
        cache.colors.resize(size_t(source.width) * source.height * 4);
        for (int y = 0; y < source.height; ++y)
            SDL_memcpy(cache.colors.data() + size_t(y) * source.width * 4,
                       static_cast<Uint8*>(pixels->pixels) + y * pixels->pitch, size_t(source.width) * 4);
        cache.image = std::move(image);
    }
    if (cache.alpha != shape.alpha) {
        auto pixels = cache.colors;
        // Rasterize the same two triangles per grid cell. This avoids seams
        // in the SDL software geometry path while retaining vertex alpha,
        // the source image's pixel alpha, and the exact partial drawing state.
        for (int y = 0; y < source.height; ++y)
            for (int x = 0; x < source.width; ++x) {
                const double u = (x + .5) * shape.segmentsX / source.width,
                             v = (y + .5) * shape.segmentsY / source.height;
                const unsigned ix = std::min(unsigned(u), shape.segmentsX - 1),
                               iy = std::min(unsigned(v), shape.segmentsY - 1);
                const double fx = u - ix, fy = v - iy;
                const size_t a = size_t(iy) * (shape.segmentsX + 1) + ix, b = a + 1,
                             c = a + shape.segmentsX + 1, d = c + 1;
                const double alpha =
                    fy <= fx ? shape.alpha[a] * (1 - fx) + shape.alpha[b] * (fx - fy) + shape.alpha[d] * fy
                             : shape.alpha[a] * (1 - fy) + shape.alpha[d] * fx + shape.alpha[c] * (fy - fx);
                const size_t index = size_t(y) * source.width + x;
                pixels[index * 4 + 3] = Uint8(std::lround(source.mask[index] * std::clamp(alpha, 0., 1.)));
            }
        if (SDL_UpdateTexture(cache.image->texture, nullptr, pixels.data(), source.width * 4) != 0)
            throw std::runtime_error(SDL_GetError());
        cache.alpha = shape.alpha;
    }
    const SDL_FRect destination{shape.sprite.x, shape.sprite.y, shape.sprite.width, shape.sprite.height};
    SDL_RenderCopyF(renderer_, cache.image->texture, nullptr, &destination);
}
void Player::drawVoodooCircle(VoodooCircle& circle) {
    SDL_RendererInfo rendererInfo{};
    SDL_GetRendererInfo(renderer_, &rendererInfo);
    struct Node {
        float z;
        const ActivityObject* sprite;
        const CircleShape* patch;
    };
    std::vector<Node> nodes;
    for (const auto& shape : circle.shapes())
        nodes.push_back({shape.sprite.z, nullptr, &shape});
    for (const auto& object : circle.objects())
        nodes.push_back({object.z, &object, nullptr});
    std::stable_sort(nodes.begin(), nodes.end(), [](const auto& a, const auto& b) { return a.z > b.z; });
    for (const auto& node : nodes) {
        if (node.sprite) {
            drawActivity({*node.sprite});
            const auto found = circle.effects().find(node.sprite->id);
            if (found != circle.effects().end() && found->second->enabled && found->second->visible) {
                drawModel(*found->second);
                drawParticles(*found->second);
            }
            continue;
        }
        const auto& shape = *node.patch;
        if (std::none_of(shape.alpha.begin(), shape.alpha.end(), [](float a) { return a > 0; }))
            continue;
        auto* texture = image(shape.sprite.texture);
        if (!texture)
            continue;
        if (rendererInfo.flags & SDL_RENDERER_SOFTWARE) {
            drawSoftwareCirclePatch(shape, *texture);
            continue;
        }
        std::vector<SDL_Vertex> vertices;
        std::vector<int> indices;
        for (unsigned y = 0; y <= shape.segmentsY; ++y)
            for (unsigned x = 0; x <= shape.segmentsX; ++x) {
                const float u = float(x) / shape.segmentsX, v = float(y) / shape.segmentsY;
                const auto alpha =
                    Uint8(std::clamp(shape.alpha[size_t(y) * (shape.segmentsX + 1) + x], 0.f, 1.f) * 255);
                vertices.push_back(
                    {{shape.sprite.x + u * shape.sprite.width, shape.sprite.y + v * shape.sprite.height},
                     {255, 255, 255, alpha},
                     {u, v}});
            }
        for (unsigned y = 0; y < shape.segmentsY; ++y)
            for (unsigned x = 0; x < shape.segmentsX; ++x) {
                const int a = int(y * (shape.segmentsX + 1) + x), b = a + 1, c = a + int(shape.segmentsX) + 1,
                          d = c + 1;
                indices.insert(indices.end(), {a, b, d, a, d, c});
            }
        SDL_SetTextureAlphaMod(texture->texture, 255);
        SDL_SetTextureBlendMode(texture->texture, SDL_BLENDMODE_BLEND);
        if (SDL_RenderGeometry(renderer_, texture->texture, vertices.data(), int(vertices.size()),
                               indices.data(), int(indices.size())) != 0)
            runtime_.report(std::string("Circle patch draw: ") + SDL_GetError());
    }
}
const ActivityObject* Player::circleTarget(float x, float y) {
    auto* circle = runtime_.voodooCircle();
    if (!circle)
        return nullptr;
    std::vector<const ActivityObject*> targets;
    for (auto it = circle->objects().rbegin(); it != circle->objects().rend(); ++it)
        if (it->interactive)
            targets.push_back(&*it);
    std::stable_sort(targets.begin(), targets.end(), [](auto* a, auto* b) { return a->z < b->z; });
    for (auto* target : targets)
        if (hitActivity(*target, x, y))
            return target;
    return nullptr;
}
void Player::clickVoodooCircle(float x, float y) {
    auto& circle = *runtime_.voodooCircle();
    if (circle.busy() || circle.drawing())
        return;
    if (inventoryClick(x, y)) {
        if (selection_ == "chalk")
            runtime_.chooseCircleChalk();
        return;
    }
    const auto* target = circleTarget(x, y);
    const auto id = target ? target->id : "";
    if (id == "exit" && selection_.empty()) {
        runtime_.closeVoodooCircle();
        return;
    }
    runtime_.interactVoodooCircle(id, selection_);
    if (!circle.drawing())
        selection_.clear();
}
void Player::drawSkullsObey(SkullsObey& skulls) {
    drawActivity(skulls.objects());
    if (skulls.eyes().enabled) {
        drawModel(skulls.eyes());
        drawParticles(skulls.eyes());
    }
}
const ActivityObject* Player::skullsTarget(float x, float y) {
    auto* skulls = runtime_.skullsObey();
    if (!skulls)
        return nullptr;
    std::vector<const ActivityObject*> targets;
    for (auto it = skulls->objects().rbegin(); it != skulls->objects().rend(); ++it)
        if (it->interactive)
            targets.push_back(&*it);
    std::stable_sort(targets.begin(), targets.end(), [](auto* a, auto* b) { return a->z < b->z; });
    for (auto* target : targets)
        if (hitActivity(*target, x, y))
            return target;
    return nullptr;
}
void Player::clickSkullsObey(float x, float y) {
    auto& skulls = *runtime_.skullsObey();
    if (skulls.busy())
        return;
    if (inventoryClick(x, y))
        return;
    if (!selection_.empty()) {
        selection_.clear();
        sound("sound/sfx/sfx_inv_item_put");
        return;
    }
    const auto* target = skullsTarget(x, y);
    const auto id = target ? target->id : "";
    if (id == "exit")
        runtime_.closeSkullsObey();
    else if (id == "wheel")
        skulls.inspectWheel();
    else if (id == "reward")
        skulls.takeReward();
    else if (id.starts_with("skull_"))
        skulls.select(size_t(std::stoul(id.substr(6))) - 1);
    else
        skulls.idleClick();
}
void Player::drawCemeteryGate(CemeteryGate& gate) {
    drawActivity(gate.objects());
    for (size_t i = 0; i < gate.blocks().size(); ++i)
        if (gate.letterVisible(i)) {
            const auto& block = gate.block(i);
            GuiElement label;
            label.kind = "label";
            label.font = gate.font();
            label.color = gate.color();
            char letter = gate.letter(i);
            if (letter >= 'a' && letter <= 'z')
                letter -= 'a' - 'A';
            label.text = std::string(1, letter);
            label.x = block.x + gate.letterX();
            label.y = block.y + gate.letterY();
            const auto size = gate.letterSize(i);
            label.width = size.first;
            label.height = size.second;
            drawGui({label});
        }
    drawParticles(gate.effect());
}
std::optional<size_t> Player::cemeteryBlockAt(float x, float y) {
    auto* gate = runtime_.cemeteryGate();
    if (!gate)
        return {};
    for (size_t i = gate->blocks().size(); i-- > 0;)
        if (hitActivity(gate->block(i), x, y))
            return i;
    return {};
}
void Player::clickCemeteryGate(float x, float y) {
    auto& gate = *runtime_.cemeteryGate();
    if (gate.busy())
        return;
    if (inventoryClick(x, y))
        return;
    if (!selection_.empty()) {
        selection_.clear();
        sound("sound/sfx/sfx_inv_item_put");
        return;
    }
    if (const auto block = cemeteryBlockAt(x, y)) {
        gate.select(*block);
        return;
    }
    if (gate.object("exit").contains(x, y))
        runtime_.closeCemeteryGate();
    else
        gate.idleClick();
}
void Player::drawBlockedWell(BlockedWell& well) {
    drawActivity(well.objects());
    for (const auto& plaque : well.plaques()) {
        GuiElement label;
        label.kind = "label";
        label.text = plaque.text;
        label.font = well.font();
        const auto color = well.color(plaque.correct);
        label.color = color;
        label.alpha = plaque.alpha;
        label.x = plaque.x;
        label.y = plaque.y;
        label.width = well.plaqueWidth();
        drawGui({label});
    }
}
std::optional<size_t> Player::wellLetterAt(float x, float y) {
    auto* well = runtime_.blockedWell();
    if (!well)
        return {};
    for (size_t i = well->alphabet().size(); i-- > 0;)
        if (hitActivity(well->letter(i), x, y))
            return i;
    return {};
}
void Player::clickBlockedWell(float x, float y) {
    auto& well = *runtime_.blockedWell();
    if (well.busy())
        return;
    hintBubble_.clear();
    if (inventoryClick(x, y))
        return;
    if (!selection_.empty()) {
        selection_.clear();
        sound("sound/sfx/sfx_inv_item_put");
        return;
    }
    if (const auto letter = wellLetterAt(x, y)) {
        well.select(*letter);
        return;
    }
    if (well.object("exit").contains(x, y))
        runtime_.closeBlockedWell();
}
bool Player::hitActivity(const ActivityObject& object, float x, float y) {
    if (!object.visible || !object.enabled || object.scale <= 0)
        return false;
    const double angle = -object.angle * 3.14159265358979323846 / 180;
    const float dx = x - object.x, dy = y - object.y;
    const float localX = float(dx * std::cos(angle) - dy * std::sin(angle)) / object.scale + object.pivotX;
    const float localY = float(dx * std::sin(angle) + dy * std::cos(angle)) / object.scale + object.pivotY;
    return hit(object.texture, 0, 0, object.width, object.height, localX, localY);
}
std::optional<size_t> Player::hexBoneAt(float x, float y) {
    auto* bones = runtime_.hexBones();
    if (!bones)
        return {};
    std::vector<size_t> candidates;
    for (size_t i = bones->current().size(); i-- > 0;)
        if (bones->bone(i).interactive)
            candidates.push_back(i);
    std::stable_sort(candidates.begin(), candidates.end(),
                     [&](auto a, auto b) { return bones->bone(a).z < bones->bone(b).z; });
    for (auto slot : candidates)
        if (hitActivity(bones->bone(slot), x, y))
            return slot;
    return {};
}
void Player::clickHexBones(float x, float y) {
    auto& bones = *runtime_.hexBones();
    hintBubble_.clear();
    if (bones.busy())
        return;
    if (hitActivity(bones.object("sack"), x, y)) {
        bones.throwBones();
        return;
    }
    if (const auto slot = hexBoneAt(x, y)) {
        bones.select(*slot);
        return;
    }
    if (bones.object("exit").contains(x, y)) {
        runtime_.closeHexBones();
        return;
    }
    if (bones.thrown() && !bones.complete()) {
        hintBubble_ = runtime_.string("puzzles/hex_bones/hints/message_scrambled");
        hintBubbleX_ = x;
        hintBubbleY_ = y;
    }
}
void Player::moveRevealBrush(int x, int y) {
    auto* mode = runtime_.revealSpirits();
    if (!mode)
        return;
    std::optional<size_t> target;
    for (size_t i = 0; i < mode->targets().size(); ++i)
        if (mode->pending(i)) {
            const auto& mask = *mode->targets()[i].mask;
            if (mask.visible && mask.enabled &&
                hit(mask.texture, mask.x, mask.y, mask.width, mask.height, float(x), float(y))) {
                target = i;
                break;
            }
        }
    mode->move(x, y, target);
}
const ActivityObject* Player::ratsTarget(float x, float y) {
    auto* rodents = runtime_.rats();
    if (!rodents)
        return nullptr;
    std::vector<const ActivityObject*> candidates;
    for (auto it = rodents->objects().rbegin(); it != rodents->objects().rend(); ++it)
        if (it->interactive && (it->id == "exit" || it->id == "seeds" || it->id.starts_with("mirror_") ||
                                it->id.starts_with("rat_")))
            candidates.push_back(&*it);
    std::stable_sort(candidates.begin(), candidates.end(), [](auto* a, auto* b) { return a->z < b->z; });
    for (const auto* object : candidates)
        if (hitActivity(*object, x, y))
            return object;
    return nullptr;
}
void Player::clickRats(float x, float y) {
    auto& rodents = *runtime_.rats();
    if (rodents.busy())
        return;
    hintBubble_.clear();
    if (inventoryClick(x, y))
        return;
    if (!selection_.empty()) {
        selection_.clear();
        return;
    }
    const auto* target = ratsTarget(x, y);
    if (target && target->id == "exit") {
        runtime_.closeRats();
        return;
    }
    if (target && target->id.starts_with("mirror_")) {
        rodents.turn(size_t(std::stoul(target->id.substr(7, 2))));
        return;
    }
    if (target && target->id == "seeds")
        rodents.idleClick(true);
    else if (target && target->id.starts_with("rat_"))
        rodents.idleClick();
}
const ActivityObject* Player::cellarTarget(float x, float y) {
    auto* door = runtime_.cellarDoors();
    if (!door)
        return nullptr;
    std::vector<const ActivityObject*> candidates;
    for (auto it = door->objects().rbegin(); it != door->objects().rend(); ++it)
        if (it->interactive &&
            (it->id == "exit" || it->id == "cellar_doors_mask" || it->id.starts_with("syllable_")))
            candidates.push_back(&*it);
    std::stable_sort(candidates.begin(), candidates.end(), [](auto* a, auto* b) { return a->z < b->z; });
    for (const auto* object : candidates)
        if (hitActivity(*object, x, y))
            return object;
    return nullptr;
}
void Player::clickCellarDoors(float x, float y) {
    auto& door = *runtime_.cellarDoors();
    if (door.busy())
        return;
    hintBubble_.clear();
    if (inventoryClick(x, y))
        return;
    const auto* target = cellarTarget(x, y);
    if (!selection_.empty()) {
        if (selection_ == "ash") {
            if (target && target->id == "cellar_doors_mask" && runtime_.applyCellarAsh())
                return;
        } else
            door.wrongItem();
        selection_.clear();
        return;
    }
    if (target && target->id == "exit") {
        runtime_.closeCellarDoors();
        return;
    }
    if (target && target->id.starts_with("syllable_")) {
        door.select(size_t(std::stoul(target->id.substr(9))));
        return;
    }
    door.idleClick();
}
std::optional<unsigned> Player::pianoKeyAt(float x, float y) {
    auto* piano = runtime_.piano();
    if (!piano || piano->busy() || piano->opened())
        return {};
    for (unsigned i = piano->maximumKey(); i >= piano->minimumKey(); --i)
        if (hitActivity(piano->key(i), x, y))
            return i;
    return {};
}
void Player::clickPiano(float x, float y) {
    auto& piano = *runtime_.piano();
    if (piano.busy())
        return;
    if (inventoryClick(x, y))
        return;
    if (!selection_.empty()) {
        selection_.clear();
        return;
    }
    if (const auto key = pianoKeyAt(x, y)) {
        piano.press(*key);
        return;
    }
    if (hitActivity(piano.object("reward"), x, y)) {
        runtime_.takePianoReward();
        return;
    }
    if (piano.object("exit").contains(x, y))
        runtime_.closePiano();
}
const ActivityObject* Player::doveTarget(float x, float y) {
    auto* cote = runtime_.doveCote();
    if (!cote)
        return nullptr;
    std::vector<const ActivityObject*> targets;
    for (auto it = cote->objects().rbegin(); it != cote->objects().rend(); ++it)
        if (it->id == "exit" || it->id == "bird_feeder" || it->id.starts_with("roll_") ||
            it->id.starts_with("lock_pipe_") || it->id.starts_with("door_open_") ||
            it->id.starts_with("door_closed_"))
            targets.push_back(&*it);
    std::stable_sort(targets.begin(), targets.end(), [](auto* a, auto* b) { return a->z < b->z; });
    for (auto* target : targets)
        if (hitActivity(*target, x, y))
            return target;
    return nullptr;
}
void Player::clickDoveCote(float x, float y) {
    auto& cote = *runtime_.doveCote();
    if (cote.busy())
        return;
    hintBubble_.clear();
    if (!cote.pulling() && inventoryClick(x, y))
        return;
    const auto* target = doveTarget(x, y);
    const std::string id = target ? target->id : "";
    if (id == "exit") {
        runtime_.closeDoveCote();
        selection_.clear();
        hintBubble_.clear();
        return;
    }
    auto hint = [&](const std::string& key) {
        hintBubble_ = runtime_.string("puzzles/dove_cote/hints/" + key);
        hintBubbleX_ = x;
        hintBubbleY_ = y;
    };
    if (cote.pulling() || selection_ == "rope") {
        if (id.starts_with("roll_")) {
            const auto index = size_t(std::stoul(id.substr(5))) - 1;
            runtime_.connectDoveRope(index);
            if (!cote.pulling())
                selection_.clear();
        } else if (!cote.pulling()) {
            hint(cote.inside() ? "wrong_item_dove_inside" : "wrong_item_no_dove");
            selection_.clear();
        }
        return;
    }
    if (!selection_.empty()) {
        if (selection_ == "seeds" && id == "bird_feeder")
            runtime_.feedDoveCote();
        else
            hint(cote.inside() ? "wrong_item_dove_inside" : "wrong_item_no_dove");
        selection_.clear();
        return;
    }
    if (id == "bird_feeder") {
        hint(cote.inside() ? "click_full_bird_feeder" : "click_empty_bird_feeder");
        return;
    }
    if (id.starts_with("lock_pipe_")) {
        cote.togglePipe(size_t(std::stoul(id.substr(10))) - 1);
        return;
    }
    if (id.starts_with("door_open_") || id.starts_with("door_closed_")) {
        cote.clickDoor(size_t(std::stoul(id.substr(id.find_last_of('_') + 1))) - 1);
        return;
    }
    hint(!cote.inside()            ? "no_dove_inside"
         : id.starts_with("roll_") ? cote.secured() ? "rope_secured_on_both_rings" : "need_rope"
                                   : "idle_click_dove_inside");
}
void Player::drawWindowCleaning(WindowCleaning& window) {
    drawActivity(window.objects());
}
std::optional<size_t> Player::windowSectionAt(float x, float y) {
    auto* window = runtime_.windowCleaning();
    if (!window)
        return {};
    for (size_t i = 10; i-- > 0;) {
        const auto& object = window->sectionObject(i);
        if (window->dirty(i) && object.visible &&
            hit(object.texture, object.x, object.y, object.width, object.height, x, y))
            return i;
    }
    return {};
}
void Player::moveWindowSponge(int x, int y) {
    auto* window = runtime_.windowCleaning();
    if (!window || !window->rubbing())
        return;
    const auto [filteredX, filteredY] = window->move(x, y);
    window->hover(windowSectionAt(float(filteredX), float(filteredY)));
    if (filteredX != x || filteredY != y) {
        int px = 0, py = 0;
        SDL_RenderLogicalToWindow(renderer_, float(filteredX), float(filteredY), &px, &py);
        SDL_WarpMouseInWindow(window_, px, py);
    }
}
void Player::clickWindowCleaning(float x, float y) {
    auto& window = *runtime_.windowCleaning();
    if (window.busy() || window.complete())
        return;
    if (inventoryClick(x, y))
        return;
    auto objectHit = [&](const char* id) {
        const auto& object = window.object(id);
        return object.visible && object.enabled &&
               hit(object.texture, object.x, object.y, object.width, object.height, x, y);
    };
    if (!selection_.empty()) {
        if (selection_ == "mixture_grease_cleaning" && objectHit("table") && runtime_.placeWindowMixture()) {
            selection_.clear();
            hint_.clear();
        } else if (selection_ == "sponge" && objectHit("cleaning_mixture") && runtime_.wetWindowSponge())
            hint_.clear();
        return;
    }
    if (window.object("exit").contains(x, y)) {
        runtime_.closeWindowCleaning();
        windowRubInput_ = false;
        return;
    }
    if (windowSectionAt(x, y))
        showHint(runtime_.string("puzzles/window_cleaning/hints/need_something_to_clean_grease"));
    else if (objectHit("table"))
        showHint(runtime_.string(std::string("puzzles/window_cleaning/hints/") +
                                 (runtime_.hasInventory("mixture_grease_cleaning")
                                      ? "need_to_put_the_bowl_on_table"
                                      : "books_can_be_used_as_table")));
}
void Player::drawCutscene(Cutscene& movie) {
    drawActivity(movie.objects());
    const std::string path = "fonts/cutscene";
    auto* face = font(path);
    for (const auto& caption : movie.captions()) {
        if (movie.elapsed() < caption.start || movie.elapsed() >= caption.end)
            continue;
        auto* measured =
            TTF_RenderUTF8_Blended_Wrapped(face, displayText(caption.text).c_str(), {255, 255, 255, 255}, 0);
        if (!measured)
            continue;
        const int x = 512 - int(std::ceil(measured->w / 2.)),
                  y = 720 - int(std::ceil(measured->h / 2.)) - TTF_FontAscent(face);
        SDL_FreeSurface(measured);
        drawText(caption.text, x, y, 0, {255, 255, 255, 255}, path);
    }
    if (movie.skippable) {
        auto controls = cutsceneControls_;
        int px = 0, py = 0;
        SDL_GetMouseState(&px, &py);
        float x = 0, y = 0;
        SDL_RenderWindowToLogical(renderer_, px, py, &x, &y);
        for (const auto& button : controls)
            if (button.id == "skip" &&
                hit(button.mask, button.x, button.y, button.width, button.height, x, y))
                for (auto& label : controls)
                    if (label.id == "skip_label")
                        label.color = 0xFFDCDCDC;
        drawGui(controls);
    }
}
bool Player::hit(const std::string& texture, float x, float y, float w, float h, float mouseX, float mouseY) {
    auto* img = image(texture);
    if (!img)
        return false;
    if (w <= 0)
        w = float(img->width);
    if (h <= 0)
        h = float(img->height);
    if (mouseX < x || mouseY < y || mouseX >= x + w || mouseY >= y + h)
        return false;
    const int u = std::clamp(int((mouseX - x) / w * img->width), 0, img->width - 1);
    const int v = std::clamp(int((mouseY - y) / h * img->height), 0, img->height - 1);
    return img->mask[static_cast<size_t>(v) * img->width + u] >= 16;
}
TTF_Font* Player::font(const std::string& path) {
    if (path.empty())
        return font_;
    const auto key = normalize(path);
    if (auto found = fonts_.find(key); found != fonts_.end())
        return found->second->face;
    auto result = std::make_unique<Font>();
    pugi::xml_document doc;
    const auto definition = assets_.read(key + ".font");
    if (!doc.load_buffer(definition.data(), definition.size()))
        throw std::runtime_error("Invalid font definition: " + key);
    const auto ttf = doc.child("font").child("ttf");
    result->bytes = assets_.read(resolve(directory(key), ttf.attribute("src").value()));
    result->face =
        TTF_OpenFontRW(SDL_RWFromConstMem(result->bytes.data(), static_cast<int>(result->bytes.size())), 1,
                       ttf.attribute("size").as_int(16));
    if (!result->face)
        throw std::runtime_error(TTF_GetError());
    auto* face = result->face;
    fonts_[key] = std::move(result);
    return face;
}
void Player::drawText(const std::string& text, int x, int y, int width, SDL_Color color,
                      const std::string& fontPath, float alpha) {
    auto* face = font(fontPath);
    if (!face || text.empty())
        return;
    auto* surface = TTF_RenderUTF8_Blended_Wrapped(face, displayText(text).c_str(), color, width);
    if (!surface)
        return;
    auto* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    if (texture)
        SDL_SetTextureAlphaMod(texture, Uint8(std::clamp(alpha, 0.f, 1.f) * 255));
    SDL_Rect rect{x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer_, texture, nullptr, &rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}
void Player::loadHud() {
    cutsceneControls_ = loadGui(assets_, "cutscenes/layout.gui");
    leftHud_ = loadGui(assets_, "hud/common/hud_left.gui");
    rightHud_ = loadGui(assets_, "hud/common/hud_right.gui");
    skipHud_ = loadGui(assets_, "hud/common/hud_skip_puzzle.gui");
    inventoryBack_ = loadGui(assets_, "hud/common/hud_inventory_graves.gui");
    for (auto& element : inventoryBack_) {
        if (element.id == "inventory_open_zone") {
            inventoryOpenZone_ = element;
            element.visible = false;
        } else
            element.visible = true;
    }
    inventorySlots_ = loadGui(assets_, "hud/common/hud_inventory_objects.gui");
    inventoryPanel_ = std::make_unique<InventoryPanel>(assets_);
    pugi::xml_document doc;
    const auto bytes = assets_.read("hobs/_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid HOB HUD configuration");
    const auto config = doc.child("hobs");
    const auto list = config.child("groupnames");
    listX_ = list.attribute("left").as_float();
    listY_ = list.attribute("top").as_float();
    listStrideX_ = list.attribute("stridex").as_float();
    listStrideY_ = list.attribute("stridey").as_float();
    listRows_ = list.attribute("colobjects").as_int();
    if (listRows_ <= 0)
        throw std::runtime_error("Invalid HOB list row count");
    const auto back = config.child("groupnamesback");
    listBackX_ = back.child("position").attribute("x").as_float();
    listBackY_ = back.child("position").attribute("y").as_float();
    listBackground_ = resolve("hobs", back.child("texture").attribute("src").value());
    listFont_ = resolve("hobs", config.child("font").attribute("src").value());
    auto color = [](const char* text) {
        const auto value = static_cast<unsigned>(std::stoul(text, nullptr, 16));
        return SDL_Color{Uint8(value >> 16), Uint8(value >> 8), Uint8(value), Uint8(value >> 24)};
    };
    listColor_ = color(config.child("font").attribute("color").value());
    listFoundColor_ = color(config.child("font").attribute("hidden_color").value());
    const auto strike = config.child("strikethrough");
    strikeTexture_ = resolve("hobs", strike.attribute("src").value());
    strikeX_ = strike.attribute("offset_x").as_float();
    strikeY_ = strike.attribute("offset_y").as_float();
    strikeExtra_ = strike.attribute("extra_length").as_float();
    strikeThickness_ = strike.attribute("thickness").as_float(1);
    const auto tooltipBytes = assets_.read("hud/common/_tooltip_config.xml");
    if (!doc.load_buffer(tooltipBytes.data(), tooltipBytes.size()))
        throw std::runtime_error("Invalid tooltip configuration");
    const auto tooltip = doc.child("const");
    tooltipFont_ = resolve("hud/common", tooltip.child("text").attribute("font").value());
    tooltipColor_ = color(tooltip.child("text").attribute("color").value());
    tooltipMarginX_ = tooltip.child("back_margin").attribute("x").as_float();
    tooltipMarginTop_ = tooltip.child("back_margin").attribute("top").as_float();
    tooltipMarginBottom_ = tooltip.child("back_margin").attribute("bottom").as_float();
    tooltipScreenWidth_ = tooltip.child("screen").attribute("width").as_float(1024);
    tooltipArrowMargin_ = tooltip.child("min_arrow_margin").attribute("x").as_float(25);
    const auto hintBytes = assets_.read("hud/common/_hint_description_config.xml");
    if (!doc.load_buffer(hintBytes.data(), hintBytes.size()))
        throw std::runtime_error("Invalid hint description configuration");
    const auto hint = doc.child("const");
    hintBack_.kind = "image";
    hintBack_.normal = resolve("hud/common", hint.child("back_texture").attribute("src").value());
    hintBack_.x = hint.child("back_position").attribute("x").as_float();
    hintBack_.y = hint.child("back_position").attribute("y").as_float();
    hintText_.kind = "label";
    hintText_.x = hintBack_.x + hint.child("back_margin").attribute("x").as_float() +
                  hint.child("start_text_offset").attribute("x").as_float();
    hintText_.y = hintBack_.y + hint.child("back_margin").attribute("y").as_float() +
                  hint.child("start_text_offset").attribute("y").as_float();
    hintText_.font = resolve("hud/common", hint.child("text").attribute("font").value());
    hintText_.color =
        static_cast<unsigned>(std::stoul(hint.child("text").attribute("color").value(), nullptr, 16));
    hintText_.width = hint.child("text").attribute("max_width").as_float();
    hintUsefulRatio_ = hint.child("back_useful_height").attribute("ratio").as_float();
    hintMaxLines_ = hint.child("text").attribute("max_lines").as_uint();
    hintBackLineX_ = hint.child("back_offset_per_extra_line_of_text").attribute("x").as_float();
    hintBackLineY_ = hint.child("back_offset_per_extra_line_of_text").attribute("y").as_float();
    hintTextLineX_ = hint.child("start_text_offset_per_extra_line_of_text").attribute("x").as_float();
    hintTextLineY_ = hint.child("start_text_offset_per_extra_line_of_text").attribute("y").as_float();
    hintMinimumSeconds_ = hint.child("min_show_time").attribute("ms").as_double() / 1000.;
    hintHideSeconds_ = hint.child("time_to_hide").attribute("ms").as_double() / 1000.;
}
void Player::showHint(const std::string& text) {
    hint_ = text;
    hintOpened_ = elapsed_;
}
bool Player::hintBottleInput(const SDL_Event& event) {
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        hintBottlePressed_ = false;
    if (menuView_ != MenuView::None || bookOpen() || hasDialog() || runtime_.hob() || runtime_.carriage() ||
        runtime_.cutscene() || runtime_.locationHintsOpen()) {
        hintBottlePressed_ = false;
        return false;
    }
    if (event.type == SDL_MOUSEMOTION && hintBottlePressed_)
        return true;
    if ((event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_MOUSEBUTTONUP) ||
        event.button.button != SDL_BUTTON_LEFT)
        return false;
    bool over = false;
    for (const auto& e : rightHud_)
        if (e.id == "hint_bottle_button" && e.visible && e.enabled &&
            hit(e.mask, e.x, e.y, e.width, e.height, float(event.button.x), float(event.button.y))) {
            over = true;
            break;
        }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        hintBottlePressed_ = over;
        return over;
    }
    const bool pressed = hintBottlePressed_;
    hintBottlePressed_ = false;
    if (pressed && over) {
        runtime_.openLocationHints();
        hint_.clear();
        if (runtime_.locationHintsOpen() && runtime_.revealSpirits())
            runtime_.revealSpirits()->pauseMotion();
    }
    return pressed || over;
}
std::string Player::locationHintButtonAt(float x, float y) {
    if (!runtime_.locationHintsOpen() || !locationHintMenu_)
        return {};
    for (const auto& e : locationHintMenu_->widgets())
        if (e.kind == "button" && e.visible && e.enabled &&
            hit(e.mask.empty() ? e.normal : e.mask, e.x, e.y, e.width, e.height, x, y))
            return e.id;
    return {};
}
void Player::drawLocationHints() {
    if (!runtime_.locationHintsOpen())
        return;
    if (!locationHintMenu_)
        locationHintMenu_ = std::make_unique<LocationHintMenu>(assets_);
    locationHintMenu_->update(runtime_.locationHintChoices(), runtime_.hintBottle().available(),
                              runtime_.hasHintHistory());
    int px = 0, py = 0;
    SDL_GetMouseState(&px, &py);
    float x = 0, y = 0;
    SDL_RenderWindowToLogical(renderer_, px, py, &x, &y);
    drawGui(locationHintMenu_->elements(locationHintButtonAt(x, y)));
}
void Player::drawHint() {
    if (hint_.empty())
        return;
    if (hintHideSeconds_ > 0 && elapsed_ - hintOpened_ >= hintHideSeconds_) {
        hint_.clear();
        return;
    }
    auto* back = image(hintBack_.normal);
    if (!back)
        return;
    auto* face = font(hintText_.font);
    auto* measured = TTF_RenderUTF8_Blended_Wrapped(face, displayText(hint_).c_str(), {255, 255, 255, 255},
                                                    Uint32(hintText_.width));
    if (!measured)
        return;
    unsigned lines = 1 + unsigned(std::max(0, measured->h - TTF_FontHeight(face))) /
                             unsigned(std::max(1, TTF_FontLineSkip(face)));
    SDL_FreeSurface(measured);
    if (hintMaxLines_ > 0)
        lines = std::min(lines, hintMaxLines_);
    const float extra = float(lines - 1);
    auto background = hintBack_, text = hintText_;
    background.x += extra * hintBackLineX_;
    background.y += extra * hintBackLineY_;
    text.x += extra * (hintBackLineX_ + hintTextLineX_);
    text.y += extra * (hintBackLineY_ + hintTextLineY_);
    text.height = (back->height - extra * hintTextLineY_) * hintUsefulRatio_;
    text.text = hint_;
    drawGui({background, text});
}
std::string Player::carriageButtonAt(float x, float y) {
    if (auto* carriage = runtime_.carriage())
        for (const auto& element : carriage->elements())
            if (element.kind == "button" && element.visible && element.enabled &&
                hit(element.mask.empty() ? element.normal : element.mask, element.x, element.y, element.width,
                    element.height, x, y))
                return element.id;
    return {};
}
void Player::drawCarriage() {
    auto* carriage = runtime_.carriage();
    if (!carriage)
        return;
    int px = 0, py = 0;
    SDL_GetMouseState(&px, &py);
    float x = 0, y = 0;
    SDL_RenderWindowToLogical(renderer_, px, py, &x, &y);
    drawGui(carriage->elements(carriageButtonAt(x, y)));
}
void Player::drawGui(const std::vector<GuiElement>& elements) {
    int mouseX = 0, mouseY = 0;
    const auto buttons = SDL_GetMouseState(&mouseX, &mouseY);
    float x = 0, y = 0;
    SDL_RenderWindowToLogical(renderer_, mouseX, mouseY, &x, &y);
    for (const auto& element : elements) {
        if (!element.visible)
            continue;
        if ((runtime_.hob() || runtime_.ouiji() || runtime_.hexBones()) && element.id == "inventory_image")
            continue;
        if ((runtime_.ouiji() || runtime_.hexBones()) &&
            (element.id == "arrow_left" || element.id == "arrow_right"))
            continue;
        if (element.id == "spell_book" && !runtime_.flag("enableSpellBook"))
            continue;
        if (element.id == "plant_book" && !runtime_.flag("enablePlantBook"))
            continue;
        auto texture = !element.enabled && !element.disabled.empty() ? element.disabled : element.normal;
        if (!element.progress.empty()) {
            const float progress =
                element.id == "hint_bottle" ? float(runtime_.hintBottle().charge() * 100) : 0;
            auto frame = element.progress.upper_bound(progress);
            if (frame != element.progress.begin())
                --frame;
            texture = frame->second;
        }
        const auto& mask = element.mask.empty() ? element.normal : element.mask;
        if (element.enabled &&
            (element.kind == "button" || element.kind == "checkbox" || element.kind == "slider") &&
            !mask.empty() && hit(mask, element.x, element.y, element.width, element.height, x, y)) {
            const auto& active = buttons & SDL_BUTTON_LMASK ? element.clicked : element.hover;
            if (!active.empty() && !textureResource(assets_, active).empty())
                texture = active;
        }
        if (!texture.empty())
            drawImage(texture, element.x, element.y, element.width, element.height, element.alpha);
        if (!element.text.empty()) {
            auto* face = font(element.font);
            const int alignment = element.horizontalAlign == "center"  ? TTF_WRAPPED_ALIGN_CENTER
                                  : element.horizontalAlign == "right" ? TTF_WRAPPED_ALIGN_RIGHT
                                                                       : TTF_WRAPPED_ALIGN_LEFT;
            TTF_SetFontWrappedAlign(face, alignment);
            const SDL_Color color{Uint8(element.color >> 16), Uint8(element.color >> 8), Uint8(element.color),
                                  Uint8(element.color >> 24)};
            auto* surface = TTF_RenderUTF8_Blended_Wrapped(face, displayText(element.text).c_str(), color,
                                                           std::max(0, int(element.width)));
            TTF_SetFontWrappedAlign(face, TTF_WRAPPED_ALIGN_LEFT);
            if (!surface)
                continue;
            // Native SceneText glyphs use negative FreeType bitmap_top (5A3950),
            // so GUI label positions are baselines, before box alignment.
            float y = element.y - (element.baseline ? TTF_FontAscent(face) : 0);
            if (element.height > 0 && element.verticalAlign == "center")
                y += (element.height - surface->h) / 2;
            else if (element.height > 0 && element.verticalAlign == "bottom")
                y += element.height - surface->h;
            SDL_Rect prior;
            SDL_RenderGetClipRect(renderer_, &prior);
            const bool clipped = SDL_RenderIsClipEnabled(renderer_);
            // Native 5A3110 emits every laid-out glyph; a label's size is not
            // a clipping rectangle. Keep an existing parent clip if present.
            if (!element.baseline && element.height > 0 && element.width > 0) {
                SDL_Rect clip{int(element.x), int(element.y), int(element.width), int(element.height)};
                if (clipped)
                    SDL_IntersectRect(&clip, &prior, &clip);
                SDL_RenderSetClipRect(renderer_, &clip);
            }
            auto* texture = SDL_CreateTextureFromSurface(renderer_, surface);
            SDL_SetTextureAlphaMod(texture, Uint8(std::clamp(element.alpha, 0.f, 1.f) * 255));
            SDL_FRect destination{element.x, y, float(surface->w), float(surface->h)};
            SDL_RenderCopyF(renderer_, texture, nullptr, &destination);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surface);
            SDL_RenderSetClipRect(renderer_, clipped ? &prior : nullptr);
        }
    }
}
void Player::drawTutorial(const std::string& text, float anchorX, float anchorY, float alpha) {
    if (alpha <= 0)
        return;
    const auto visible = displayText(text);
    auto* face = font(tooltipFont_);
    const int wrapWidth = 0; // Native tooltip text preserves authored line breaks.
    auto* measured = TTF_RenderUTF8_Blended_Wrapped(face, visible.c_str(), tooltipColor_, wrapWidth);
    if (!measured)
        return;
    const std::string base = "hud/common/tips/tips_";
    auto* topArrow = image(base + "bt");
    auto* bottomArrow = image(base + "bb");
    if (!topArrow || !bottomArrow) {
        SDL_FreeSurface(measured);
        return;
    }
    const float minimumWidth =
        std::max(98.f, std::max(topArrow->width, bottomArrow->width) + 2 * tooltipArrowMargin_);
    const float width = std::max(minimumWidth, measured->w + 2 * tooltipMarginX_),
                height = std::max(90.f, measured->h + tooltipMarginTop_ + tooltipMarginBottom_);
    SDL_FreeSurface(measured);
    float left = anchorX - width / 2;
    if (left < 0)
        left = 0;
    else if (left + width > tooltipScreenWidth_)
        left = tooltipScreenWidth_ - width;
    // Native 468810 flips below the anchor when the frame and arrow do not fit.
    const bool below = height + std::max(topArrow->height, bottomArrow->height) >= anchorY;
    const auto* arrow = below ? topArrow : bottomArrow;
    const float top = below ? anchorY + topArrow->height - 45 : anchorY - height - bottomArrow->height + 45;
    const float arrowX = std::clamp(anchorX - arrow->width / 2.f, left + tooltipArrowMargin_,
                                    left + width - arrow->width - tooltipArrowMargin_);
    const float middleWidth = width - 98, middleHeight = height - 90;
    drawImage(base + "ul", left, top, 0, 0, alpha);
    drawImage(base + "uc", left + 49, top, middleWidth, 45, alpha);
    drawImage(base + "ur", left + width - 49, top, 0, 0, alpha);
    drawImage(base + "cl", left, top + 45, 49, middleHeight, alpha);
    drawImage(base + "texture", left + 49, top + 45, middleWidth, middleHeight, alpha);
    drawImage(base + "cr", left + width - 49, top + 45, 49, middleHeight, alpha);
    drawImage(base + "bl", left, top + height - 45, 0, 0, alpha);
    drawImage(base + "bc", left + 49, top + height - 45, middleWidth, 45, alpha);
    drawImage(base + "br", left + width - 49, top + height - 45, 0, 0, alpha);
    drawImage(base + (below ? "bt" : "bb"), arrowX, below ? anchorY : top + height - 45, 0, 0, alpha);
    drawText(text, int(left + tooltipMarginX_), int(top + tooltipMarginTop_), wrapWidth, tooltipColor_,
             tooltipFont_, alpha);
}
void Player::drawHobHud(const HiddenObjectRound& round) {
    drawImage(listBackground_, listBackX_, listBackY_);
    for (size_t i = 0; i < round.targets.size(); ++i) {
        const auto& item = round.items[round.targets[i]];
        const float x = listX_ + float(i / listRows_) * listStrideX_,
                    y = listY_ + float(i % listRows_) * listStrideY_;
        // The HOB list uses a baseline without a GUI alignment rectangle.
        const float top = y - TTF_FontAscent(font(listFont_));
        drawText(item.listText(), int(x), int(top), int(listStrideX_),
                 item.found() ? listFoundColor_ : listColor_, listFont_);
        if (item.found()) {
            int width = 0;
            TTF_SizeUTF8(font(listFont_), displayText(item.name).c_str(), &width, nullptr);
            drawImage(strikeTexture_, x + strikeX_, y + strikeY_, width + strikeExtra_, strikeThickness_);
        }
    }
    if (runtime_.hobFeedback().fadingOut())
        drawImage("common/black", 0, 0, 1024, 768, runtime_.hobFeedback().fadeAlpha());
    drawInventory();
    if (!inventoryOpen_) {
        if (!hobInventoryHint_.empty())
            drawTutorial(hobInventoryHint_, hobInventoryHintX_, hobInventoryHintY_);
    }
}
void Player::drawTeaching() {
    if (tutorialHint_.empty() || hasDialog())
        return;
    if (tutorialId_ && tutorialSpec(*tutorialId_).picture)
        drawImage(tutorialHint_, tutorialX_, tutorialY_);
    else
        drawTutorial(tutorialHint_, tutorialX_, tutorialY_);
}
int Player::sound(const std::string& name, int loops, AudioBus bus) {
    if (!audio_ || name.empty())
        return -1;
    const auto path = normalize(name) + (name.ends_with(".ogg") ? "" : ".ogg");
    auto found = sounds_.find(path);
    if (found == sounds_.end()) {
        if (!assets_.contains(path)) {
            runtime_.report("Missing audio: " + path);
            return -1;
        }
        const auto bytes = assets_.read(path);
        auto* chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(bytes.data(), static_cast<int>(bytes.size())), 1);
        if (!chunk) {
            runtime_.report(std::string("Audio decode: ") + Mix_GetError());
            return -1;
        }
        found = sounds_.emplace(path, chunk).first;
    }
    const auto channel = Mix_GroupAvailable(-1);
    if (channel < 0) {
        runtime_.report("No free audio channel");
        return -1;
    }
    if (normalize(name).starts_with("music/"))
        bus = AudioBus::Music;
    // Set gain before playback, including the first mixed audio buffer.
    Mix_Volume(channel, int(std::lround(settings_->volume(bus) * MIX_MAX_VOLUME)));
    if (Mix_PlayChannel(channel, found->second, loops) < 0) {
        runtime_.report(Mix_GetError());
        return -1;
    }
    channelBuses_[channel] = bus;
    return channel;
}
void Player::applyVolumes() {
    if (audio_)
        for (auto [channel, bus] : channelBuses_)
            Mix_Volume(channel, int(std::lround(settings_->volume(bus) * MIX_MAX_VOLUME)));
}
void Player::changeWindowMode() {
    const bool next = !settings_->windowed();
    if (SDL_SetWindowFullscreen(window_, next ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP) == 0) {
        settings_->windowed(next);
        settings_->save();
    } else {
        optionsMenu_->windowAvailable(false);
        SDL_ClearError();
    }
}
void Player::processEvents() {
    if (tutorialHint_.empty() && !inventoryTutorials_.empty()) {
        runtime_.events.push_front(std::move(inventoryTutorials_.front()));
        inventoryTutorials_.pop_front();
    }
    while (!runtime_.events.empty()) {
        auto event = std::move(runtime_.events.front());
        runtime_.events.pop_front();
        auto arg = [&](size_t n) { return n < event.args.size() ? event.args[n] : std::string{}; };
        if (event.type == "inventoryAdded") {
            if (menuView_ == MenuView::None && !runtime_.cutscene() && !bookOpen() && !runtime_.carriage() &&
                !runtime_.revealSpirits() && inventoryImages_.contains(arg(0))) {
                if (!inventoryFeedback_)
                    inventoryFeedback_ = std::make_unique<InventoryFeedback>(assets_);
                inventoryFeedback_->start(arg(0), inventoryPointerX_, inventoryPointerY_);
            } else
                inventoryOffset_ = 0;
        } else if (event.type == "endGame") {
            saveRequested_ = true;
            openMenu(MenuView::Credits);
        } else if (event.type == "spiritLockFinalSound") {
            spiritLockSoundChannel_ = sound(arg(0));
            if (spiritLockSoundChannel_ >= 0 && runtime_.spiritLock()) {
                spiritLockSound_ = Mix_GetChunk(spiritLockSoundChannel_);
                runtime_.spiritLock()->useAudioCallback(true);
            }
        } else if (event.type == "playSound" || event.type == "playEnvSound")
            sound(arg(0), 0, event.type == "playEnvSound" ? AudioBus::Environment : AudioBus::Effects);
        else if (event.type == "skullsWheelSound") {
            const bool owned =
                audio_ && skullsWheelChannel_ >= 0 && Mix_GetChunk(skullsWheelChannel_) == skullsWheelSound_;
            if (owned)
                Mix_HaltChannel(skullsWheelChannel_);
            skullsWheelChannel_ = -1;
            skullsWheelSound_ = nullptr;
            if (!arg(0).empty()) {
                skullsWheelChannel_ = sound(arg(0), -1);
                if (audio_ && skullsWheelChannel_ >= 0)
                    skullsWheelSound_ = Mix_GetChunk(skullsWheelChannel_);
            }
        } else if (event.type == "circleChalkSound") {
            const bool owned =
                audio_ && circleChalkChannel_ >= 0 && Mix_GetChunk(circleChalkChannel_) == circleChalkSound_;
            if (arg(0).empty()) {
                if (owned) {
                    if (runtime_.voodooCircle() && runtime_.voodooCircle()->drawing())
                        Mix_Pause(circleChalkChannel_);
                    else {
                        Mix_HaltChannel(circleChalkChannel_);
                        circleChalkChannel_ = -1;
                        circleChalkSound_ = nullptr;
                    }
                }
            } else if (owned && Mix_Playing(circleChalkChannel_))
                Mix_Resume(circleChalkChannel_);
            else {
                circleChalkChannel_ = sound(arg(0), -1);
                if (audio_ && circleChalkChannel_ >= 0)
                    circleChalkSound_ = Mix_GetChunk(circleChalkChannel_);
            }
        } else if (event.type == "activityAmbient" || event.type == "revealBrushSound") {
            // SDL may recycle a stopped channel for another sound. Only stop
            // it when it still belongs to this activity's loop.
            if (audio_ && activityAmbientChannel_ >= 0 &&
                Mix_GetChunk(activityAmbientChannel_) == activityAmbientSound_)
                Mix_HaltChannel(activityAmbientChannel_);
            activityAmbientChannel_ = -1;
            activityAmbientSound_ = nullptr;
            if (!arg(0).empty()) {
                activityAmbientChannel_ = sound(arg(0), -1, AudioBus::Environment);
                if (audio_ && activityAmbientChannel_ >= 0)
                    activityAmbientSound_ = Mix_GetChunk(activityAmbientChannel_);
            }
        } else if (event.type == "cutsceneSound") {
            if (runtime_.cutscene())
                sound(arg(0));
        } else if (event.type == "selectInventoryObject") {
            selection_ = runtime_.hasInventory(arg(0)) ? arg(0) : std::string{};
            if (event.origin == "skip_puzzle")
                cancelPointerInput();
        } else if (event.type == "showHint" || event.type == "locationHintText")
            showHint(arg(0));
        else if (event.type == "showHintBubble") {
            hintBubble_ = arg(0);
            hintBubbleX_ = lastInteractionX_;
            hintBubbleY_ = lastInteractionY_;
        } else if (event.type == "showTutorialHint") {
            if (event.args.size() >= 4) {
                const auto id = std::stoi(arg(3));
                if (id >= int(TutorialId::FirstItem) && id <= int(TutorialId::InventoryPin) &&
                    !tutorialHint_.empty()) {
                    inventoryTutorials_.push_back(std::move(event));
                    continue;
                }
            }
            tutorialHint_ = arg(0);
            tutorialX_ = event.args.size() >= 3 ? std::stof(arg(1)) : lastInteractionX_;
            tutorialY_ = event.args.size() >= 3 ? std::stof(arg(2)) : lastInteractionY_;
            tutorialId_.reset();
            if (event.args.size() >= 4) {
                const int id = std::stoi(arg(3));
                if (id < 0 || id >= int(TutorialId::Count))
                    throw std::runtime_error("Invalid tutorial ID");
                tutorialId_ = static_cast<TutorialId>(id);
            }
            hintBubble_.clear();
            hobInventoryHint_.clear();
            sound("sound/sfx/sfx_hint");
        } else if (event.type == "clearHintBubble") {
            hintBubble_.clear();
            clearTutorial();
        } else if (event.type == "beginRevealSpirits") {
            revealWasInventoryOpen_ = inventoryOpen_;
            inventoryPanel_->reset();
            inventoryAllowed_ = false;
            syncInventoryPanel();
            selection_ = arg(0);
            revealPressed_ = false;
            revealPotPressed_ = false;
        } else if (event.type == "endRevealSpirits") {
            inventoryPanel_->reset(revealWasInventoryOpen_);
            inventoryAllowed_ = true;
            syncInventoryPanel();
            suppressRevealMouseUp_ = revealPressed_;
            revealPressed_ = false;
            revealPotPressed_ = false;
        } else if (event.type == "showMessageDialog" || event.type == "showMessageDialogWithImage" ||
                   event.type == "showMessageDialogWithSound" || event.type == "showTalkingFaceDialog" ||
                   event.type == "showDialog" || event.type == "showConfirmationDialog") {
            dialogs_.emplace_back(event);
        } else if (event.type == "log" || event.type == "logWarning" || event.type == "logError")
            std::cerr << event.type << ": " << arg(0) << '\n';
        else {
            runtime_.report("Presentation command not implemented: " + event.type);
        }
    }
}
bool Player::hobListAt(float x, float y) {
    return hit(listBackground_, listBackX_, listBackY_, 0, 0, x, y);
}
void Player::pressHobTarget(float x, float y) {
    if (x < 0 || y < 0 || x >= 1024 || y >= 768 || inventoryClick(x, y, false))
        return;
    for (const auto* hud : {&leftHud_, &rightHud_}) {
        for (const auto& element : *hud) {
            const auto& id = element.id;
            const bool control = id == "button_menu" || id == "hint_bottle_button" ||
                                 (id == "spell_book" && runtime_.flag("enableSpellBook")) ||
                                 (id == "plant_book" && runtime_.flag("enablePlantBook"));
            if (control && element.visible && element.enabled &&
                hit(element.mask.empty() ? element.normal : element.mask, element.x, element.y, element.width,
                    element.height, x, y))
                return;
        }
    }
    auto* round = runtime_.hob();
    lastInteractionX_ = x;
    lastInteractionY_ = y;
    hobInventoryHint_.clear();
    auto hits = [&](const HobPiece& piece) {
        return piece.visible && piece.interactive && !piece.found &&
               hit(piece.texture, piece.x, piece.y, piece.width, piece.height, x, y);
    };
    const HobPiece* target = nullptr;
    size_t targetItem = 0, targetPiece = 0;
    for (size_t index : round->targets) {
        const auto& item = round->items[index];
        for (size_t part = 0; part < item.pieces.size(); ++part) {
            const auto& piece = item.pieces[part];
            if ((!target || piece.z < target->z) && hits(piece)) {
                target = &piece;
                targetItem = index;
                targetPiece = part;
            }
        }
    }
    if (target) {
        auto covers = [&](const HobPiece& piece) { return piece.z < target->z && hits(piece); };
        const bool blocked = std::any_of(round->background.begin(), round->background.end(), covers) ||
                             std::any_of(round->items.begin(), round->items.end(), [&](const HobItem& item) {
                                 return !item.target && item.environment &&
                                        std::any_of(item.pieces.begin(), item.pieces.end(), covers);
                             });
        if (!blocked) {
            if (!runtime_.pickHob(targetItem, targetPiece, selection_) && selection_.empty() &&
                !target->inventory.empty()) {
                runtime_.hobToolHint(targetItem, targetPiece);
                hobInventoryHint_ = target->inventoryMessage;
                hobInventoryHintX_ = x;
                hobInventoryHintY_ = y;
            }
            return;
        }
    }
    runtime_.missHob(x, y);
}
void Player::click(float x, float y, bool right) {
    lastInteractionX_ = x;
    lastInteractionY_ = y;
    if (runtime_.hobFeedback().busy() && !hasDialog())
        return;
    if (runtime_.voodooCircle() && runtime_.voodooCircle()->busy())
        return;
    if (runtime_.cutscene()) {
        if (!right && runtime_.cutscene()->skippable)
            for (const auto& element : cutsceneControls_)
                if (element.id == "skip" && element.visible && element.enabled) {
                    const auto& mask = element.mask.empty() ? element.normal : element.mask;
                    if (hit(mask, element.x, element.y, element.width, element.height, x, y)) {
                        runtime_.skipCutscene();
                        break;
                    }
                }
        return;
    }
    if (hasDialog()) {
        if (right)
            return;
        if (!dialogs_.front().started)
            return;
        auto elements = dialogLayouts_.at(dialogs_.front().kind);
        for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
            if (it->kind != "button" || it->id == "shortcut_continue")
                continue;
            if (it->id == "button_ok" && dialogs_.front().page + 1 < dialogs_.front().pages.size())
                continue;
            if (it->id == "button_next" && dialogs_.front().page + 1 == dialogs_.front().pages.size())
                continue;
            const auto& mask = it->mask.empty() ? it->normal : it->mask;
            if (!mask.empty() && hit(mask, it->x, it->y, it->width, it->height, x, y)) {
                dialogInput(it->id);
                return;
            }
        }
        return;
    }
    if (runtime_.carriage()) {
        if (!right) {
            const auto button = carriageButtonAt(x, y);
            if (!button.empty() && button == carriagePressedButton_)
                runtime_.chooseCarriage(button);
        }
        carriagePressedButton_.clear();
        return;
    }
    if (runtime_.locationHintsOpen()) {
        if (!right) {
            const auto button = locationHintButtonAt(x, y);
            if (!button.empty() && button == locationHintPressedButton_) {
                if (button == "button_hints_close")
                    runtime_.closeLocationHints();
                else if (button == "button_view_activated_hints")
                    runtime_.replayLocationHint();
                else if (auto choice = locationHintMenu_->choices().find(button);
                         choice != locationHintMenu_->choices().end())
                    runtime_.takeLocationHint(choice->second);
            } else
                for (const auto& e : rightHud_)
                    if (e.id == "hint_bottle_button" && hit(e.mask, e.x, e.y, e.width, e.height, x, y))
                        runtime_.closeLocationHints();
        }
        locationHintPressedButton_.clear();
        return;
    }
    if (spellBookOpen_) {
        if (right)
            return;
        const auto* button = spellBookButtonAt(x, y);
        const bool activate = button && button == spellBookPressed_ && spellBookButtonEnabled(*button);
        spellBookPressed_ = nullptr;
        if (activate)
            spellBookAction(button->id);
        return;
    }
    if (runtime_.revealSpirits()) {
        if (right) {
            runtime_.closeRevealSpirits();
            hintBubble_.clear();
        }
        return;
    }
    if (auto* shelves = runtime_.apothecary(); shelves && (shelves->busy() || shelves->sheetOpen())) {
        if (!shelves->busy() && !right)
            shelves->toggleSheet();
        return;
    }
    if (auto* cloth = runtime_.sail(); cloth && cloth->busy())
        return;
    if (auto* skulls = runtime_.skullsObey(); skulls && skulls->busy())
        return;
    if (!hint_.empty() && elapsed_ - hintOpened_ >= hintMinimumSeconds_)
        hint_.clear();
    if (!right && runtime_.flag("enablePlantBook"))
        for (const auto& element : rightHud_) {
            if (element.id == "plant_book" &&
                hit(element.mask, element.x, element.y, element.width, element.height, x, y)) {
                togglePlantBook();
                return;
            }
        }
    if (!right && runtime_.flag("enableSpellBook"))
        for (const auto& element : leftHud_) {
            if (element.id == "spell_book" &&
                hit(element.mask, element.x, element.y, element.width, element.height, x, y)) {
                toggleSpellBook();
                return;
            }
        }
    if (right) {
        if (auto* circle = runtime_.voodooCircle(); circle && circle->drawing()) {
            circle->cancelDrawing();
            circleStrokeInput_ = false;
        }
        if (auto* worms = runtime_.leeches(); worms && worms->holding()) {
            worms->refuseCancel();
            return;
        }
        if (auto* shelves = runtime_.apothecary())
            shelves->cancel();
        if (auto* cote = runtime_.doveCote(); cote && cote->pulling()) {
            cote->undo();
            selection_ = cote->pulling() ? "rope" : "";
            hintBubble_.clear();
            return;
        }
        if (auto* window = runtime_.windowCleaning())
            window->cancel();
        if (!selection_.empty())
            sound("sound/sfx/sfx_inv_item_put");
        windowRubInput_ = false;
        selection_.clear();
        mixHeld_.clear();
        mixMortarHeld_ = false;
        voodooHeld_.clear();
        voodooHint_.clear();
        hint_.clear();
        return;
    }
    if (runtime_.windowCleaning()) {
        clickWindowCleaning(x, y);
        return;
    }
    if (runtime_.hexBones()) {
        clickHexBones(x, y);
        return;
    }
    if (runtime_.piano()) {
        clickPiano(x, y);
        return;
    }
    if (runtime_.doveCote()) {
        clickDoveCote(x, y);
        return;
    }
    if (runtime_.cellarDoors()) {
        clickCellarDoors(x, y);
        return;
    }
    if (runtime_.rats()) {
        clickRats(x, y);
        return;
    }
    if (runtime_.spiritLock()) {
        clickSpiritLock(x, y);
        return;
    }
    if (runtime_.blockedWell()) {
        clickBlockedWell(x, y);
        return;
    }
    if (runtime_.skullsObey()) {
        clickSkullsObey(x, y);
        return;
    }
    if (runtime_.cemeteryGate()) {
        clickCemeteryGate(x, y);
        return;
    }
    if (runtime_.voodooCircle()) {
        clickVoodooCircle(x, y);
        return;
    }
    if (runtime_.apothecary()) {
        clickApothecary(x, y);
        return;
    }
    if (runtime_.sail()) {
        clickSail(x, y);
        return;
    }
    if (runtime_.leeches()) {
        clickLeeches(x, y);
        return;
    }
    if (runtime_.voodoo()) {
        clickVoodoo(x, y);
        return;
    }
    if (runtime_.mixing()) {
        clickMixing(x, y);
        return;
    }
    if (auto* board = runtime_.ouiji()) {
        std::vector<const ActivityObject*> sorted;
        for (const auto& object : board->objects())
            sorted.push_back(&object);
        std::reverse(sorted.begin(), sorted.end());
        std::stable_sort(sorted.begin(), sorted.end(), [](auto* a, auto* b) { return a->z < b->z; });
        for (auto* object : sorted)
            if (object->visible && object->interactive && object->contains(x, y) &&
                board->select(object->id)) {
                if (board->moving())
                    sound("sound/sfx/mg/mg_board_move");
                if (board->exitRequested)
                    runtime_.closeOuiji();
                return;
            }
        return;
    }
    if (runtime_.hob()) {
        for (const auto& element : rightHud_) {
            if (element.id != "hint_bottle_button" ||
                !hit(element.mask, element.x, element.y, element.width, element.height, x, y))
                continue;
            runtime_.requestHobHint([&](const HobPiece& piece) {
                const auto* texture = image(piece.texture);
                if (!texture)
                    throw std::runtime_error("Missing hinted HOB texture: " + piece.texture);
                return std::pair{piece.x + (piece.width > 0 ? piece.width : texture->width) / 2,
                                 piece.y + (piece.height > 0 ? piece.height : texture->height) / 2};
            });
            return;
        }
        if (inventoryClick(x, y))
            return;
        if (hobListAt(x, y)) {
            selection_.clear();
            runtime_.closeHob();
        }
        return;
    }
    if (inventoryClick(x, y))
        return;
    if (runtime_.layers().empty())
        return;
    if (!selection_.empty() && runtime_.applyAnywhere(selection_)) {
        selection_.clear();
        sound("sound/sfx/sfx_inv_item_put");
        return;
    }
    auto sorted = runtime_.findScene(runtime_.layers().back())->objects;
    std::reverse(sorted.begin(), sorted.end());
    std::stable_sort(sorted.begin(), sorted.end(), [](auto* a, auto* b) { return a->z < b->z; });
    for (auto* object : sorted) {
        if (!object->enabled || !object->visible || !object->interactive || object->states.empty())
            continue;
        if (!hit(object->texture, object->x, object->y, object->width, object->height, x, y))
            continue;
        if (!selection_.empty()) {
            runtime_.callback(*object, "onInventoryApplied", selection_);
            selection_.clear();
            sound("sound/sfx/sfx_inv_item_put");
        } else
            runtime_.callback(*object, "onLMouseUp");
        runtime_.flushNavigation();
        return;
    }
}
void Player::startDialog() {
    if (!hasDialog() || dialogs_.front().started)
        return;
    auto& dialog = dialogs_.front();
    if (!dialogLayouts_.contains(dialog.kind))
        dialogLayouts_[dialog.kind] = loadGui(assets_, "dialogs/" + dialog.kind + "/layout.gui");
    dialog.started = true;
    dialogOpened_ = elapsed_;
    dialog.voiceChannel = sound(dialog.voice);
}
bool Player::dialogInput(const std::string& action) {
    if (!hasDialog() || !dialogs_.front().started)
        return false;
    auto& dialog = dialogs_.front();
    if (dialog.kind == "confirmation" && action != "button_yes" && action != "button_no")
        return false;
    if (action == "anywhere" &&
        elapsed_ - dialogOpened_ <
            (dialog.kind == "talking_face" ? talkingAnyClickSeconds_ : dialogAnyClickSeconds_))
        return false;
    if (!dialog.next())
        return true;
    auto closed = std::move(dialog);
    dialogs_.pop_front();
    if (audio_ && closed.voiceChannel >= 0)
        Mix_HaltChannel(closed.voiceChannel);
    runtime_.finishDialog(closed.after, closed.origin,
                          closed.kind == "confirmation" ? (action == "button_yes" ? "yes" : "no") : "ok");
    processEvents();
    return true;
}
void Player::drawDialog() {
    if (!hasDialog())
        return;
    const auto& dialog = dialogs_.front();
    auto elements = dialogLayouts_.at(dialog.kind);
    for (auto& element : elements) {
        if (element.id == "prompt")
            element.text = dialog.pages[dialog.page].text;
        if (element.id == "image") {
            element.normal = dialog.image;
            element.visible = !dialog.image.empty();
        }
        if (element.id == "button_next")
            element.visible = dialog.page + 1 < dialog.pages.size();
        if (element.id == "button_ok")
            element.visible = dialog.page + 1 == dialog.pages.size();
        if (!element.alphaKeys.empty()) {
            const auto time = elapsed_ - dialogOpened_;
            const auto next = element.alphaKeys.upper_bound(time);
            if (next == element.alphaKeys.begin())
                element.alpha = next->second;
            else if (next == element.alphaKeys.end())
                element.alpha = element.alphaKeys.rbegin()->second;
            else {
                const auto previous = std::prev(next);
                const auto ratio = float((time - previous->first) / (next->first - previous->first));
                element.alpha = previous->second + ratio * (next->second - previous->second);
            }
        }
    }
    drawGui(elements);
}
void Player::drawCursor() {
    if (SDL_GetMouseFocus() != window_)
        return;
    int mouseX = 0, mouseY = 0;
    SDL_GetMouseState(&mouseX, &mouseY);
    float x = 0, y = 0;
    SDL_RenderWindowToLogical(renderer_, mouseX, mouseY, &x, &y);
    if (menuView_ == MenuView::None && runtime_.hobFeedback().busy()) {
        const auto cursor = runtime_.hobFeedback().penalized() ? "lock" : "idle";
        if (runtime_.hobFeedback().penalized()) {
            const auto position = runtime_.hobFeedback().penaltyPosition();
            x = float(position.first);
            y = float(position.second);
        }
        const auto spot = cursorSpots_.at(cursor);
        drawImage(std::string("cursors/") + cursor, x - spot.first, y - spot.second);
        return;
    }
    if (menuView_ != MenuView::None || plantBookOpen_ || runtime_.carriage() ||
        runtime_.locationHintsOpen()) {
        const auto spot = cursorSpots_.at("idle");
        drawImage("cursors/idle", x - spot.first, y - spot.second);
        return;
    }
    if (x < 0 || y < 0 || x >= 1024 || y >= 768)
        return;
    if (runtime_.revealSpirits() && !bookOpen() && !hasDialog()) {
        const auto spot = cursorSpots_.at("reveal_spirits");
        drawImage("cursors/reveal_spirits", x - spot.first, y - spot.second);
        return;
    }
    std::string cursor = "idle";
    if (runtime_.hob() && !bookOpen() && !hasDialog() && !inventoryClick(x, y, false) && hobListAt(x, y))
        cursor = "go_down";
    if (runtime_.leeches() && !bookOpen() && !hasDialog()) {
        if (const auto* target = leechesTarget(x, y))
            cursor = target->id == "exit" ? "go_down" : "interact";
    }
    if (auto* cloth = runtime_.sail(); cloth && !bookOpen() && !hasDialog()) {
        if (cloth->busy())
            cursor = "wait";
        else if (cloth->gripping() || (!cloth->ready() && sailPieceAt(x, y)))
            cursor = "interact";
        else if (y > cloth->exitY())
            cursor = "go_down";
    }
    if (auto* shelves = runtime_.apothecary(); shelves && !bookOpen() && !hasDialog()) {
        if (shelves->busy())
            cursor = "wait";
        else if (const auto* target = apothecaryTarget(x, y))
            cursor = target->id == "exit" ? "go_down" : target->id == "sheet" ? "view" : "interact";
    }
    if (auto* well = runtime_.blockedWell(); well && !well->busy() && !bookOpen() && !hasDialog()) {
        if (wellLetterAt(x, y))
            cursor = "interact";
        else if (well->object("exit").contains(x, y))
            cursor = "go_down";
    }
    if (auto* skulls = runtime_.skullsObey(); skulls && !bookOpen() && !hasDialog()) {
        if (skulls->busy())
            cursor = "wait";
        else if (const auto* target = skullsTarget(x, y)) {
            if (target->id == "exit")
                cursor = "go_down";
            else if (target->id == "reward")
                cursor = "interact_taking";
            else if (target->id == "wheel" || target->id.starts_with("skull_"))
                cursor = "interact";
        }
    }
    if (auto* board = runtime_.ouiji(); board && !bookOpen() && !hasDialog()) {
        if (board->busy())
            cursor = "wait";
        else if (board->object("exit").contains(x, y))
            cursor = "go_down";
        else
            for (const auto& o : board->objects())
                if (o.visible && o.interactive && o.contains(x, y)) {
                    if ((board->opened() && !board->complete() && o.id == "back_no_board") ||
                        (!board->solved() && (o.id == "yes" || o.id == "no" || o.id == "goodbye" ||
                                              (o.id.starts_with("smb") && o.id.size() == 6))))
                        cursor = "interact";
                }
    }
    if (auto* gate = runtime_.cemeteryGate(); gate && !bookOpen() && !hasDialog()) {
        if (gate->busy())
            cursor = "wait";
        else if (cemeteryBlockAt(x, y))
            cursor = "interact";
        else if (gate->object("exit").contains(x, y))
            cursor = "go_down";
    }
    if (auto* circle = runtime_.voodooCircle(); circle && !bookOpen() && !hasDialog()) {
        if (circle->busy())
            cursor = "wait";
        else if (const auto* target = circleTarget(x, y))
            cursor = target->id == "exit" ? "go_down" : "interact";
    }
    if (auto* lock = runtime_.spiritLock(); lock && !bookOpen() && !hasDialog()) {
        if (lock->turning())
            cursor = "interact";
        else if (!lock->busy()) {
            const auto* target = spiritLockTarget(x, y);
            if (target && target->id == "exit")
                cursor = "go_down";
            else if (target && lock->assembled())
                cursor = "interact";
        }
    }
    if (!runtime_.cutscene() && !runtime_.windowCleaning() && !runtime_.voodoo() && !runtime_.hexBones() &&
        !runtime_.piano() && !runtime_.doveCote() && !runtime_.cellarDoors() && !runtime_.rats() &&
        !runtime_.spiritLock() && !runtime_.blockedWell() && !runtime_.cemeteryGate() &&
        !runtime_.skullsObey() && !runtime_.voodooCircle() && !runtime_.apothecary() && !runtime_.sail() &&
        !runtime_.leeches() && !bookOpen() && !runtime_.hob() && !runtime_.ouiji() && !runtime_.mixing() &&
        !hasDialog() && (!inventoryOpen_ || y < inventoryBack_.at(1).y) && !runtime_.layers().empty()) {
        auto sorted = runtime_.findScene(runtime_.layers().back())->objects;
        std::reverse(sorted.begin(), sorted.end());
        std::stable_sort(sorted.begin(), sorted.end(), [](auto* a, auto* b) { return a->z < b->z; });
        for (auto* object : sorted) {
            if (!object->enabled || !object->visible || !object->interactive || object->states.empty())
                continue;
            if (hit(object->texture, object->x, object->y, object->width, object->height, x, y)) {
                if (cursorSpots_.contains(object->cursor) &&
                    !textureResource(assets_, "cursors/" + object->cursor).empty())
                    cursor = object->cursor;
                break;
            }
        }
    }
    if (auto* window = runtime_.windowCleaning(); window && (window->rubbing() || window->busy()))
        return;
    if (auto* bones = runtime_.hexBones(); bones && !bookOpen() && !hasDialog()) {
        if (!bones->busy()) {
            if (hexBoneAt(x, y) || hitActivity(bones->object("sack"), x, y))
                cursor = "interact";
            else if (bones->object("exit").contains(x, y))
                cursor = "go_down";
        }
    }
    if (auto* piano = runtime_.piano(); piano && !bookOpen() && !hasDialog()) {
        if (piano->busy())
            cursor = "wait";
        else if (pianoKeyAt(x, y) || hitActivity(piano->object("reward"), x, y))
            cursor = "interact";
        else if (piano->object("exit").contains(x, y))
            cursor = "go_down";
    }
    if (auto* cote = runtime_.doveCote(); cote && !bookOpen() && !hasDialog()) {
        if (cote->busy())
            cursor = "wait";
        else if (const auto* target = doveTarget(x, y))
            cursor = target->id == "exit" ? "go_down" : "interact";
    }
    if (auto* door = runtime_.cellarDoors(); door && !bookOpen() && !hasDialog()) {
        if (door->busy())
            cursor = "wait";
        else if (const auto* target = cellarTarget(x, y))
            cursor = target->id == "exit" ? "go_down" : "interact";
    }
    if (auto* rodents = runtime_.rats(); rodents && !bookOpen() && !hasDialog()) {
        if (rodents->busy())
            cursor = "wait";
        else if (const auto* target = ratsTarget(x, y))
            cursor = target->id == "exit" ? "go_down" : "interact";
    }
    if (mixMortarHeld_ && runtime_.mixing()) {
        const auto path = inventoryImages_.at("plate");
        if (auto* img = image(path))
            drawImage(path, x - img->width / 2.f, y - img->height / 2.f);
        return;
    }
    if (!mixHeld_.empty() && runtime_.mixing()) {
        const auto& ingredient = runtime_.mixing()->ingredients().at(mixHeld_);
        const auto id = ingredient.cursorObject.empty() ? ingredient.inventory : ingredient.cursorObject;
        const auto found = inventoryImages_.find(id);
        if (found == inventoryImages_.end()) {
            runtime_.report("Missing mixing inventory cursor: " + id);
            return;
        }
        if (auto* img = image(found->second))
            drawImage(found->second, x - img->width / 2.f, y - img->height / 2.f);
        return;
    }
    const std::string leechCursor =
        runtime_.leeches() && runtime_.leeches()->holding() && !bookOpen() && !hasDialog()
            ? "orange_with_leech"
            : "";
    const auto& inventoryCursor = !leechCursor.empty()                        ? leechCursor
                                  : !voodooHeld_.empty() && runtime_.voodoo() ? voodooHeld_
                                                                              : selection_;
    if (!inventoryCursor.empty() && inventoryImages_.contains(inventoryCursor) &&
        !(runtime_.hob() && cursor == "go_down")) {
        auto* img = image(inventoryImages_.at(inventoryCursor));
        if (img)
            drawImage(inventoryImages_.at(inventoryCursor), x - img->width / 2.f, y - img->height / 2.f);
        return;
    }
    const auto spot = cursorSpots_.at(cursor);
    drawImage("cursors/" + cursor, x - spot.first, y - spot.second);
}
void Player::render() {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    if (menuView_ != MenuView::None) {
        drawMenu();
        return;
    }
    renderWorld();
}
void Player::renderWorld() {
    if (auto* movie = runtime_.cutscene()) {
        drawCutscene(*movie);
        return;
    }
    if (auto* window = runtime_.windowCleaning())
        drawWindowCleaning(*window);
    else if (auto* bones = runtime_.hexBones())
        drawActivity(bones->objects());
    else if (auto* piano = runtime_.piano())
        drawActivity(piano->objects());
    else if (auto* cote = runtime_.doveCote())
        drawActivity(cote->objects());
    else if (auto* door = runtime_.cellarDoors())
        drawActivity(door->objects());
    else if (auto* rodents = runtime_.rats())
        drawActivity(rodents->objects());
    else if (auto* lock = runtime_.spiritLock())
        drawSpiritLock(*lock);
    else if (auto* well = runtime_.blockedWell())
        drawBlockedWell(*well);
    else if (auto* skulls = runtime_.skullsObey())
        drawSkullsObey(*skulls);
    else if (auto* gate = runtime_.cemeteryGate())
        drawCemeteryGate(*gate);
    else if (auto* circle = runtime_.voodooCircle())
        drawVoodooCircle(*circle);
    else if (auto* shelves = runtime_.apothecary())
        drawApothecary(*shelves);
    else if (auto* cloth = runtime_.sail())
        drawActivity(cloth->objects());
    else if (auto* worms = runtime_.leeches())
        drawActivity(worms->objects());
    else if (auto* doll = runtime_.voodoo())
        drawVoodoo(*doll);
    else if (auto* mix = runtime_.mixing())
        drawMixing(*mix);
    else if (auto* board = runtime_.ouiji())
        drawOuiji(*board);
    else if (auto* round = runtime_.hob())
        drawHob(*round);
    else
        for (const auto& layer : runtime_.layers())
            drawScene(*runtime_.findScene(layer));
    if (runtime_.hobFeedback().returning())
        drawImage("common/black", 0, 0, 1024, 768, runtime_.hobFeedback().fadeAlpha());
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    if (runtime_.ouiji() || runtime_.hexBones()) {
        drawGui(leftHud_);
        drawGui(rightHud_);
    } else if (auto* round = runtime_.hob()) {
        drawHobHud(*round);
    } else {
        drawInventory();
    }
    drawHint();
    if ((runtime_.hexBones() || runtime_.doveCote() || runtime_.cellarDoors() || runtime_.rats() ||
         runtime_.spiritLock() || runtime_.blockedWell() || runtime_.apothecary() ||
         runtime_.revealSpirits()) &&
        !hintBubble_.empty())
        drawTutorial(hintBubble_, hintBubbleX_, hintBubbleY_);
    if (auto* shelves = runtime_.apothecary(); shelves && !shelves->busy() && !shelves->sheetOpen() &&
                                               hintBubble_.empty() && !hasDialog() && !bookOpen() &&
                                               !inventoryOpen_) {
        int px = 0, py = 0;
        SDL_GetMouseState(&px, &py);
        float x = 0, y = 0;
        SDL_RenderWindowToLogical(renderer_, px, py, &x, &y);
        if (const auto* target = apothecaryTarget(x, y)) {
            std::optional<size_t> item;
            if (target->id.starts_with("item_"))
                item = size_t(std::stoul(target->id.substr(5))) - 1;
            else if (target->id.starts_with("shelf_"))
                item = shelves->slots().at(size_t(std::stoul(target->id.substr(6))) - 1);
            if (item)
                drawTutorial(shelves->items().at(*item).tooltip, x, y);
        }
    }
    if (runtime_.voodoo() && !inventoryOpen_ && !voodooHint_.empty())
        drawTutorial(voodooHint_, voodooHintX_, voodooHintY_);
    drawCarriage();
    drawLocationHints();
    drawSpellBook();
    drawPlantBook();
    drawPuzzleSkip();
    drawTeaching();
    drawDialog();
}
int Player::run(const PlayerOptions& options) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | (options.audio ? SDL_INIT_AUDIO : 0)) != 0)
        throw std::runtime_error(SDL_GetError());
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    if (TTF_Init() != 0)
        throw std::runtime_error(TTF_GetError());
    window_ = SDL_CreateWindow("Deadtime Stories", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768,
                               SDL_WINDOW_RESIZABLE);
    if (!window_)
        throw std::runtime_error(SDL_GetError());
    if (!settings_->windowed() && SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
        settings_->windowed(true);
        SDL_ClearError();
    }
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_)
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_)
        throw std::runtime_error(SDL_GetError());
    SDL_RenderSetLogicalSize(renderer_, 1024, 768);
    loadHud();
    pugi::xml_document game;
    const auto gameBytes = assets_.read("deadtime.game");
    if (!game.load_buffer(gameBytes.data(), gameBytes.size()))
        throw std::runtime_error("Invalid game configuration");
    inventoryDragMs_ =
        game.child("deadtime").child("drag_and_drop").attribute("start_drag_time_ms").as_uint(300);
    const auto delays = game.child("deadtime").child("dialogs").child("min_time_to_close_by_any_click");
    dialogAnyClickSeconds_ = delays.attribute("default_ms").as_double(2000) / 1000.;
    talkingAnyClickSeconds_ = delays.child("talking_face").attribute("ms").as_double(3000) / 1000.;
    pugi::xml_document cursors;
    const auto cursorBytes = assets_.read("cursors/cursors.xml");
    if (!cursors.load_buffer(cursorBytes.data(), cursorBytes.size()))
        throw std::runtime_error("Invalid cursor configuration");
    for (auto cursor : cursors.child("cursors").children())
        cursorSpots_[cursor.name()] = {cursor.attribute("spotX").as_float(),
                                       cursor.attribute("spotY").as_float()};
    if (!cursorSpots_.contains("idle"))
        throw std::runtime_error("Missing idle cursor");
    previousCursor_ = SDL_ShowCursor(SDL_QUERY);
    SDL_ShowCursor(SDL_DISABLE);
    if (assets_.contains("fonts/times.ttf")) {
        fontBytes_ = assets_.read("fonts/times.ttf");
        font_ =
            TTF_OpenFontRW(SDL_RWFromConstMem(fontBytes_.data(), static_cast<int>(fontBytes_.size())), 1, 20);
        if (!font_)
            runtime_.report(TTF_GetError());
    }
    if (assets_.contains("inventory/_objects.xml")) {
        pugi::xml_document doc;
        const auto data = assets_.read("inventory/_objects.xml");
        doc.load_buffer(data.data(), data.size());
        for (auto node : doc.document_element().children()) {
            const auto id = node.name();
            if (*id) {
                inventoryImages_[id] = resolve("inventory", node.attribute("texture").value());
                inventoryTooltips_[id] = node.attribute("tooltip").value();
            }
        }
    }
    if (options.audio) {
        Mix_Init(MIX_INIT_OGG);
        audio_ = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) == 0;
        if (!audio_)
            runtime_.report(Mix_GetError());
        else
            Mix_AllocateChannels(32);
    }
    if (options.profileMenu)
        openMenu(MenuView::Profiles);
    else if (options.credits)
        openMenu(MenuView::Credits);
    else if (options.menu)
        openMenu(MenuView::Main);
    auto last = SDL_GetTicks64();
    int frame = 0;
    while (!quit_) {
        if (options.frameInput)
            options.frameInput();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEMOTION) {
                inventoryPointerX_ = float(event.motion.x);
                inventoryPointerY_ = float(event.motion.y);
            } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
                inventoryPointerX_ = float(event.button.x);
                inventoryPointerY_ = float(event.button.y);
            }
            if (event.type == SDL_RENDER_DEVICE_RESET) {
                images_.clear();
                circlePatchImages_.clear();
            }
            if (saves_ && event.type == SDL_MOUSEBUTTONDOWN && event.button.button > 0 &&
                event.button.button < 32) {
                pointerButtons_ |= SDL_BUTTON(event.button.button);
                suppressPointerUp_ = false;
            }
            if (saves_ && event.type == SDL_MOUSEBUTTONUP && event.button.button > 0 &&
                event.button.button < 32) {
                const bool held = (pointerButtons_ & SDL_BUTTON(event.button.button)) != 0;
                pointerButtons_ &= ~SDL_BUTTON(event.button.button);
                if (!held || suppressPointerUp_) {
                    suppressPointerUp_ = false;
                    continue;
                }
            }
            if (saves_ && event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                cancelPointerInput();
                saveRequested_ = true;
            }
            if (event.type == SDL_QUIT) {
                if (saves_)
                    cancelPointerInput();
                quit_ = true;
            }
            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT &&
                menuView_ == MenuView::None)
                clearTutorial();
            if (menuInput(event))
                continue;
            if (runtime_.hobFeedback().busy() && !hasDialog() &&
                (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP ||
                 event.type == SDL_MOUSEMOTION || event.type == SDL_KEYDOWN))
                continue;
            if (confirmationInput(event))
                continue;
            if (plantBookInput(event))
                continue;
            if (puzzleSkipInput(event))
                continue;
            if (hintBottleInput(event))
                continue;
            if (event.type == SDL_MOUSEBUTTONDOWN && runtime_.hob() && !hasDialog() && !bookOpen() &&
                !runtime_.cutscene()) {
                try {
                    if (event.button.button == SDL_BUTTON_LEFT)
                        pressHobTarget(float(event.button.x), float(event.button.y));
                    else if (event.button.button == SDL_BUTTON_RIGHT && !selection_.empty()) {
                        selection_.clear();
                        sound("sound/sfx/sfx_inv_item_put");
                    }
                } catch (const std::exception& error) {
                    runtime_.report(error.what());
                }
                continue;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                locationHintPressedButton_.clear();
                carriagePressedButton_.clear();
                spellBookPressed_ = nullptr;
                if (auto* window = runtime_.windowCleaning())
                    window->cancel();
                windowRubInput_ = false;
                activityPressed_ = false;
                doveRightPressed_ = false;
                revealPotPressed_ = false;
                revealPressed_ = false;
                if (runtime_.revealSpirits())
                    runtime_.revealSpirits()->pauseMotion();
                if (spiritLockGrip_ && runtime_.spiritLock())
                    runtime_.spiritLock()->release();
                if (runtime_.sail())
                    runtime_.sail()->cancelGrip();
                if (runtime_.voodooCircle())
                    runtime_.voodooCircle()->release();
                circleStrokeInput_ = false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT &&
                runtime_.carriage() && !hasDialog())
                carriagePressedButton_ = carriageButtonAt(float(event.button.x), float(event.button.y));
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT &&
                runtime_.locationHintsOpen() && !hasDialog())
                locationHintPressedButton_ =
                    locationHintButtonAt(float(event.button.x), float(event.button.y));
            if (runtime_.locationHintsOpen() && !hasDialog()) {
                // Keep every puzzle's press, drag and hover behind this modal.
                if (event.type == SDL_MOUSEBUTTONUP)
                    click(float(event.button.x), float(event.button.y),
                          event.button.button == SDL_BUTTON_RIGHT);
                continue;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT &&
                spellBookOpen_ && !hasDialog()) {
                spellBookPressed_ = spellBookButtonAt(float(event.button.x), float(event.button.y));
                continue;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
                activityPressed_ =
                    (runtime_.ouiji() || runtime_.hexBones() || runtime_.piano() || runtime_.doveCote() ||
                     runtime_.cellarDoors() || runtime_.rats() || runtime_.spiritLock() ||
                     runtime_.blockedWell() || runtime_.cemeteryGate() || runtime_.skullsObey() ||
                     runtime_.voodooCircle() || runtime_.apothecary() || runtime_.sail() ||
                     runtime_.leeches()) &&
                    !hasDialog() && !bookOpen() && (!runtime_.ouiji() || !runtime_.ouiji()->busy()) &&
                    (!runtime_.hexBones() || !runtime_.hexBones()->busy()) &&
                    (!runtime_.piano() || !runtime_.piano()->busy()) &&
                    (!runtime_.doveCote() || !runtime_.doveCote()->busy()) &&
                    (!runtime_.cellarDoors() || !runtime_.cellarDoors()->busy()) &&
                    (!runtime_.rats() || !runtime_.rats()->busy()) &&
                    (!runtime_.spiritLock() || !runtime_.spiritLock()->busy()) &&
                    (!runtime_.blockedWell() || !runtime_.blockedWell()->busy()) &&
                    (!runtime_.skullsObey() || !runtime_.skullsObey()->busy()) &&
                    (!runtime_.cemeteryGate() || !runtime_.cemeteryGate()->busy()) &&
                    (!runtime_.voodooCircle() || !runtime_.voodooCircle()->busy()) &&
                    (!runtime_.apothecary() || !runtime_.apothecary()->busy()) &&
                    (!runtime_.sail() || !runtime_.sail()->busy()) &&
                    (!runtime_.leeches() || !runtime_.leeches()->complete());
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
                doveRightPressed_ =
                    runtime_.doveCote() && !runtime_.doveCote()->busy() && !hasDialog() && !bookOpen();
            if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                if (runtime_.cutscene()) {
                    const auto key = event.key.keysym.sym;
                    if (key == SDLK_ESCAPE || key == SDLK_SPACE || key == SDLK_RETURN)
                        runtime_.skipCutscene();
                } else if (hasDialog()) {
                    const auto key = event.key.keysym.sym;
                    if (key == SDLK_RETURN || key == SDLK_SPACE ||
                        (key == SDLK_ESCAPE && dialogs_.front().kind != "talking_face"))
                        dialogInput("button_ok");
                } else if (auto* carriage = runtime_.carriage()) {
                    const auto key = event.key.keysym.sym;
                    const auto name = key == SDLK_RETURN || key == SDLK_KP_ENTER ? "enter"
                                      : key == SDLK_ESCAPE                       ? "esc"
                                                                                 : "";
                    const auto button = carriage->shortcut(name);
                    if (!button.empty())
                        runtime_.chooseCarriage(button);
                    carriagePressedButton_.clear();
                } else if (runtime_.locationHintsOpen()) {
                    // The original selection GUI declares no keyboard shortcuts.
                } else if (spellBookOpen_) {
                    const auto key = event.key.keysym.sym;
                    if (key == SDLK_ESCAPE || key == SDLK_s)
                        spellBookAction("close");
                    else if (key == SDLK_p)
                        togglePlantBook();
                    else if (key == SDLK_r || key == SDLK_PAGEDOWN)
                        spellBookAction("recipes");
                    else if (key == SDLK_i || key == SDLK_PAGEUP)
                        spellBookAction("instructions");
                    else if (key == SDLK_LEFT)
                        spellBookAction("prev_page");
                    else if (key == SDLK_RIGHT)
                        spellBookAction("next_page");
                } else if (event.key.keysym.sym == SDLK_p) {
                    togglePlantBook();
                } else if (event.key.keysym.sym == SDLK_s) {
                    toggleSpellBook();
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (runtime_.revealSpirits()) {
                        runtime_.closeRevealSpirits();
                        hintBubble_.clear();
                    } else if (runtime_.windowCleaning()) {
                        runtime_.closeWindowCleaning();
                        windowRubInput_ = false;
                        selection_.clear();
                    } else if (runtime_.hexBones()) {
                        runtime_.closeHexBones();
                        hintBubble_.clear();
                        selection_.clear();
                    } else if (runtime_.piano()) {
                        runtime_.closePiano();
                        selection_.clear();
                    } else if (runtime_.doveCote()) {
                        runtime_.closeDoveCote();
                        selection_.clear();
                        hintBubble_.clear();
                    } else if (runtime_.cellarDoors()) {
                        if (!runtime_.cellarDoors()->busy()) {
                            runtime_.closeCellarDoors();
                            selection_.clear();
                            hintBubble_.clear();
                        }
                    } else if (runtime_.rats()) {
                        if (!runtime_.rats()->busy()) {
                            runtime_.closeRats();
                            selection_.clear();
                            hintBubble_.clear();
                        }
                    } else if (runtime_.spiritLock()) {
                        runtime_.closeSpiritLock();
                        selection_.clear();
                        hintBubble_.clear();
                    } else if (runtime_.blockedWell()) {
                        runtime_.closeBlockedWell();
                        selection_.clear();
                        hintBubble_.clear();
                    } else if (runtime_.skullsObey()) {
                        if (!runtime_.skullsObey()->busy()) {
                            runtime_.closeSkullsObey();
                            selection_.clear();
                        }
                    } else if (runtime_.cemeteryGate()) {
                        if (!runtime_.cemeteryGate()->busy()) {
                            runtime_.closeCemeteryGate();
                            selection_.clear();
                        }
                    } else if (runtime_.voodooCircle()) {
                        if (!runtime_.voodooCircle()->busy()) {
                            circleSuppressMouseUp_ = circleStrokeInput_;
                            runtime_.closeVoodooCircle();
                            circleStrokeInput_ = false;
                            selection_.clear();
                        }
                    } else if (runtime_.apothecary()) {
                        runtime_.closeApothecary();
                        selection_.clear();
                        hintBubble_.clear();
                    } else if (runtime_.sail()) {
                        if (!runtime_.sail()->busy()) {
                            runtime_.closeSail();
                            selection_.clear();
                            hintBubble_.clear();
                        }
                    } else if (runtime_.leeches()) {
                        if (runtime_.leeches()->holding())
                            runtime_.leeches()->requireSwamp();
                        else if (!runtime_.leeches()->complete()) {
                            runtime_.closeLeeches();
                            selection_.clear();
                        }
                    } else if (runtime_.voodoo()) {
                        runtime_.closeVoodoo();
                        voodooHeld_.clear();
                        voodooHint_.clear();
                        selection_.clear();
                    } else if (runtime_.mixing()) {
                        runtime_.mixing()->releaseStir();
                        runtime_.closeMixing();
                        mixHeld_.clear();
                        mixMortarHeld_ = false;
                        mixStirInput_ = false;
                    } else if (runtime_.ouiji())
                        runtime_.closeOuiji();
                    else if (runtime_.hob())
                        runtime_.closeHob();
                    else if (saves_)
                        pauseMenu();
                    else
                        quit_ = true;
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT &&
                runtime_.revealSpirits() && !hasDialog() && !bookOpen()) {
                revealPressed_ = true;
                const auto& pot = runtime_.revealSpirits()->container();
                revealPotPressed_ = hit(pot.texture, pot.x, pot.y, pot.width, pot.height,
                                        float(event.button.x), float(event.button.y));
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT &&
                !runtime_.locationHintsOpen() && !runtime_.carriage() && !runtime_.cutscene() &&
                !bookOpen() && !hasDialog() && !runtime_.hob() && !runtime_.ouiji() &&
                (!runtime_.skullsObey() || !runtime_.skullsObey()->busy()) &&
                (!runtime_.sail() || !runtime_.sail()->busy()) &&
                (!runtime_.leeches() || !runtime_.leeches()->holding()) &&
                (!runtime_.voodooCircle() ||
                 (!runtime_.voodooCircle()->drawing() && !runtime_.voodooCircle()->busy()))) {
                pressedInventory_ = inventoryItemAt(float(event.button.x), float(event.button.y));
                inventoryPressedAt_ = SDL_GetTicks64();
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT &&
                !runtime_.cutscene() && !hasDialog() && !bookOpen()) {
                if (auto* circle = runtime_.voodooCircle(); circle && circle->drawing()) {
                    circle->press(float(event.button.x), float(event.button.y));
                    circleStrokeInput_ = true;
                    pressedInventory_.clear();
                }
                if (auto* cloth = runtime_.sail();
                    cloth && selection_.empty() &&
                    (!inventoryOpen_ || event.button.y < inventoryBack_.at(1).y)) {
                    if (const auto piece = sailPieceAt(float(event.button.x), float(event.button.y));
                        piece && cloth->press(*piece, float(event.button.x), float(event.button.y)))
                        pressedInventory_.clear();
                }
                if (auto* lock = runtime_.spiritLock();
                    lock && selection_.empty() &&
                    (!inventoryOpen_ || event.button.y < inventoryBack_.at(1).y)) {
                    const auto* target = spiritLockTarget(float(event.button.x), float(event.button.y));
                    if (target && (target->id == "lock" || target->id == "all_pieces")) {
                        spiritLockGrip_ = lock->press(float(event.button.x), float(event.button.y));
                        if (spiritLockGrip_) {
                            pressedInventory_.clear();
                            hintBubble_.clear();
                        }
                    }
                }
                if (runtime_.windowCleaning() && (selection_ == "sponge" || selection_ == "sponge_wet")) {
                    const auto section = windowSectionAt(float(event.button.x), float(event.button.y));
                    if (section &&
                        runtime_.beginWindowCleaning(*section, selection_, event.button.x, event.button.y)) {
                        windowRubInput_ = true;
                        pressedInventory_.clear();
                    }
                }
                if (auto* mix = runtime_.mixing(); mix && mix->readyToStir()) {
                    const auto& pestle = mix->object(mix->sceneObjects().at("pestle_in_mortar"));
                    if (hit(pestle.texture, pestle.x, pestle.y, pestle.width, pestle.height,
                            float(event.button.x), float(event.button.y))) {
                        mixGrabX_ = event.button.x - pestle.x;
                        mixGrabY_ = event.button.y - pestle.y;
                        mixStirInput_ =
                            mix->startStir(mixAngle(float(event.button.x), float(event.button.y)));
                        pressedInventory_.clear();
                    }
                }
            }
            if (event.type == SDL_MOUSEMOTION && windowRubInput_)
                moveWindowSponge(event.motion.x, event.motion.y);
            if (event.type == SDL_MOUSEMOTION && runtime_.voodooCircle() && !hasDialog() && !bookOpen())
                runtime_.voodooCircle()->move(float(event.motion.x), float(event.motion.y));
            if (event.type == SDL_MOUSEMOTION && runtime_.sail() && !hasDialog() && !bookOpen())
                runtime_.sail()->move(float(event.motion.x), float(event.motion.y));
            if (event.type == SDL_MOUSEMOTION && runtime_.blockedWell() && !hasDialog() && !bookOpen())
                runtime_.blockedWell()->hover(wellLetterAt(float(event.motion.x), float(event.motion.y)));
            if (event.type == SDL_MOUSEMOTION && spiritLockGrip_ && runtime_.spiritLock() && !hasDialog() &&
                !bookOpen())
                runtime_.spiritLock()->move(float(event.motion.x), float(event.motion.y));
            if (event.type == SDL_MOUSEMOTION && runtime_.revealSpirits() && !hasDialog() && !bookOpen())
                moveRevealBrush(event.motion.x, event.motion.y);
            if (event.type == SDL_MOUSEMOTION && mixStirInput_)
                moveMixPestle(float(event.motion.x), float(event.motion.y));
            if (event.type == SDL_MOUSEMOTION && runtime_.doveCote() && !bookOpen() && !hasDialog())
                runtime_.doveCote()->moveRope(float(event.motion.x), float(event.motion.y));
            if (event.type == SDL_MOUSEMOTION && !draggingInventory_ && !mixStirInput_ &&
                !pressedInventory_.empty() && SDL_GetTicks64() - inventoryPressedAt_ >= inventoryDragMs_) {
                selection_ = pressedInventory_;
                draggingInventory_ = true;
                sound("sound/sfx/sfx_inv_item_pick");
            }
            if (event.type == SDL_MOUSEBUTTONUP) {
                try {
                    if (event.button.button == SDL_BUTTON_LEFT &&
                        tryHudMenu(float(event.button.x), float(event.button.y))) {
                    } else if (circleSuppressMouseUp_ && event.button.button == SDL_BUTTON_LEFT)
                        circleSuppressMouseUp_ = false;
                    else if (circleStrokeInput_ && event.button.button == SDL_BUTTON_LEFT) {
                        if (runtime_.voodooCircle())
                            runtime_.voodooCircle()->release();
                        circleStrokeInput_ = false;
                    } else if (runtime_.sail() && runtime_.sail()->gripping() &&
                               event.button.button == SDL_BUTTON_LEFT)
                        runtime_.sail()->release();
                    else if (spiritLockGrip_ && event.button.button == SDL_BUTTON_LEFT) {
                        if (runtime_.spiritLock())
                            runtime_.spiritLock()->release();
                        spiritLockGrip_ = false;
                    } else if (suppressRevealMouseUp_ && event.button.button == SDL_BUTTON_LEFT) {
                        suppressRevealMouseUp_ = false;
                    } else if (runtime_.revealSpirits()) {
                        if (event.button.button == SDL_BUTTON_RIGHT)
                            runtime_.closeRevealSpirits();
                        else if (event.button.button == SDL_BUTTON_LEFT && revealPotPressed_) {
                            const auto& pot = runtime_.revealSpirits()->container();
                            if (hit(pot.texture, pot.x, pot.y, pot.width, pot.height, float(event.button.x),
                                    float(event.button.y)))
                                runtime_.closeRevealSpirits();
                        }
                        revealPressed_ = false;
                        revealPotPressed_ = false;
                    } else if (windowRubInput_) {
                        if (auto* window = runtime_.windowCleaning())
                            window->cancel(event.button.button == SDL_BUTTON_LEFT);
                        if (event.button.button == SDL_BUTTON_RIGHT)
                            selection_.clear();
                        windowRubInput_ = false;
                    } else if (mixStirInput_) {
                        if (runtime_.mixing())
                            runtime_.mixing()->releaseStir();
                        mixStirInput_ = false;
                    } else if (draggingInventory_ && inventoryOpen_ &&
                               event.button.y >= inventorySlots_.front().y) {
                        selection_.clear();
                        sound("sound/sfx/sfx_inv_item_put");
                    } else if (runtime_.doveCote() && event.button.button == SDL_BUTTON_RIGHT &&
                               !doveRightPressed_ && !hasDialog() && !bookOpen()) {
                    } else if ((!runtime_.ouiji() && !runtime_.hexBones() && !runtime_.piano() &&
                                !runtime_.doveCote() && !runtime_.cellarDoors() && !runtime_.rats() &&
                                !runtime_.spiritLock() && !runtime_.blockedWell() &&
                                !runtime_.cemeteryGate() && !runtime_.skullsObey() &&
                                !runtime_.voodooCircle() && !runtime_.apothecary() && !runtime_.sail() &&
                                !runtime_.leeches()) ||
                               event.button.button != SDL_BUTTON_LEFT || activityPressed_ || hasDialog() ||
                               bookOpen())
                        click(float(event.button.x), float(event.button.y),
                              event.button.button == SDL_BUTTON_RIGHT);
                } catch (const std::exception& e) {
                    runtime_.report(e.what());
                    showHint("This action is not implemented yet.");
                }
                pressedInventory_.clear();
                draggingInventory_ = false;
                activityPressed_ = false;
                doveRightPressed_ = false;
            }
        }
        const auto now = SDL_GetTicks64();
        const auto delta = std::min(0.1, (now - last) / 1000.);
        if (menuView_ == MenuView::None && spiritLockSoundChannel_ >= 0) {
            if (!runtime_.spiritLock())
                spiritLockSoundChannel_ = -1;
            else if (!Mix_Playing(spiritLockSoundChannel_) ||
                     Mix_GetChunk(spiritLockSoundChannel_) != spiritLockSound_) {
                runtime_.spiritLock()->finishPronunciation();
                spiritLockSoundChannel_ = -1;
            }
        }
        if (menuView_ != MenuView::Pause) {
            elapsed_ += delta;
            animationTime_ += delta;
        }
        if (menuView_ == MenuView::None && !runtime_.locationHintsOpen())
            if (auto* mode = runtime_.revealSpirits(); mode && mode->position())
                moveRevealBrush(mode->position()->first, mode->position()->second);
        runtime_.setSkipBookOpen(bookOpen());
        if (menuView_ == MenuView::None) {
            runtime_.tick(delta);
            if (inventoryFeedback_) {
                if (runtime_.cutscene() || bookOpen() || runtime_.carriage() || runtime_.revealSpirits())
                    inventoryFeedback_->clear();
                else {
                    if (!inventoryFeedback_->tick(delta).empty())
                        inventoryOffset_ = 0;
                    const auto page = inventoryPage();
                    for (const auto& flight : inventoryFeedback_->flights())
                        if (flight->arrived && !flight->feedbackFinished &&
                            std::find(page.begin(), page.end(), flight->item) == page.end())
                            inventoryFeedback_->finishLabel(flight->item);
                    for (const auto& item : inventoryFeedback_->takeCompletions()) {
                        (void)item;
                        runtime_.inventoryFeedbackFinished();
                    }
                }
            }
        } else if (menuView_ == MenuView::Credits) {
            credits_->tick(delta);
            if (credits_->complete())
                openMenu(MenuView::Main);
        } else if (menuScene_) {
            for (auto* object : menuScene_->scene().objects)
                runtime_.tickVisual(*object, delta);
        }
        if (menuView_ == MenuView::None && runtime_.hobFeedback().penalized() &&
            SDL_GetMouseFocus() == window_) {
            const auto position = runtime_.hobFeedback().penaltyPosition();
            int x = 0, y = 0;
            SDL_RenderLogicalToWindow(renderer_, float(position.first), float(position.second), &x, &y);
            SDL_WarpMouseInWindow(window_, x, y);
        }
        last = now;
        if (cutsceneWasActive_ != (runtime_.cutscene() != nullptr) ||
            currentCutsceneGeneration_ != runtime_.cutsceneGeneration()) {
            if (audio_)
                Mix_HaltChannel(-1);
            currentScene_.clear();
            selection_.clear();
            mixHeld_.clear();
            mixMortarHeld_ = false;
            mixStirInput_ = false;
            windowRubInput_ = false;
            currentCutsceneGeneration_ = runtime_.cutsceneGeneration();
            cutsceneWasActive_ = runtime_.cutscene() != nullptr;
        }
        if (menuView_ == MenuView::None && !runtime_.cutscene() && !runtime_.layers().empty() &&
            runtime_.layers().front() != currentScene_) {
            currentScene_ = runtime_.layers().front();
            if (audio_)
                Mix_HaltChannel(-1);
            const auto* scene = runtime_.findScene(currentScene_);
            if (!scene->music.empty())
                sound(scene->music.front(), -1);
            sound(scene->ambient, -1, AudioBus::Environment);
        }
        processEvents();
        if (menuView_ == MenuView::None) {
            updateInventory();
            inventoryPanel_->tick(delta);
            syncInventoryPanel();
        }
        if (menuView_ == MenuView::None && !runtime_.cutscene()) {
            startDialog();
            if (hasDialog())
                dialogs_.front().tick(delta);
        }
        if (saves_)
            saveProgress(quit_);
        render();
        drawCursor();
        ++frame;
        if (options.frames && frame >= options.frames)
            quit_ = true;
        if (quit_ && !options.snapshot.empty()) {
            int w = 0, h = 0;
            SDL_GetRendererOutputSize(renderer_, &w, &h);
            auto* frameSurface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
            if (!frameSurface)
                throw std::runtime_error(SDL_GetError());
            const int read = SDL_RenderReadPixels(renderer_, nullptr, SDL_PIXELFORMAT_RGBA32,
                                                  frameSurface->pixels, frameSurface->pitch);
            const int save = read == 0 ? IMG_SavePNG(frameSurface, options.snapshot.c_str()) : -1;
            SDL_FreeSurface(frameSurface);
            if (save != 0)
                throw std::runtime_error("Could not save frame: " + options.snapshot);
        }
        SDL_RenderPresent(renderer_);
    }
    if (saves_)
        saveProgress(true);
    return runtime_.errors().empty() ? (reloadProfile_ ? ReloadProfile : restartGame_ ? RestartGame : 0) : 2;
}
} // namespace deadtime
