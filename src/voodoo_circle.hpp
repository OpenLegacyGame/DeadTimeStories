#pragma once
#include "animation_set.hpp"
#include <map>
#include <memory>
#include <optional>

namespace deadtime {
struct Object;
struct CircleEvent {
    enum Kind { Sound, ChalkSound, Hint, PopupHint, Award, Consume, Select, Flash, Finish } kind;
    std::string value;
};
struct CircleItem {
    std::string inventory;
    bool onScene = true, conduit = false;
    unsigned state = 0;
    std::optional<size_t> slot;
};
struct CircleShape {
    ActivityObject sprite;
    unsigned segmentsX = 0, segmentsY = 0;
    std::vector<std::pair<float, float>> points;
    std::vector<bool> painted;
    std::vector<float> alpha;
    double distance = 0, stopDistance = 0, required = 0, strokeLength = 0, strokeThickness = 0;
    bool complete = false;
    size_t paintedCount() const;
};
class VoodooCircle {
    friend class ActivityCheckpoints;

  public:
    VoodooCircle(const Assets& assets, const std::string& directory);
    ~VoodooCircle();
    void enter(bool spellKnown);
    void leave();
    void setSpellKnown(bool known) {
        spellKnown_ = known;
    }
    bool clearGarbage(size_t index);
    bool takeItem(size_t index);
    bool takeMoss();
    bool chooseChalk();
    bool apply(const std::string& inventory, std::optional<size_t> slot);
    bool skip();
    void idleClick(bool hasChalk);
    void press(float x, float y);
    void move(float x, float y);
    void release();
    void cancelDrawing();
    void tick(double seconds);
    bool cleared() const;
    bool drawn() const;
    bool drawing() const {
        return drawing_;
    }
    bool pressing() const {
        return pressing_;
    }
    bool busy() const {
        return ending_ && !complete_;
    }
    bool complete() const {
        return complete_;
    }
    bool entered() const {
        return entered_;
    }
    bool mossTaken() const {
        return moss_;
    }
    const auto& garbage() const {
        return garbage_;
    }
    const auto& items() const {
        return items_;
    }
    const auto& shapes() const {
        return shapes_;
    }
    const auto& objects() const {
        return objects_;
    }
    const auto& spell() const {
        return spell_;
    }
    auto& effects() {
        return effects_;
    }
    const ActivityObject& object(const std::string& id) const;
    std::vector<CircleEvent> takeEvents();
    std::string onEnter, onExit, onFinish;
    bool rewarded = false;

  private:
    std::string base_, spell_;
    std::vector<ActivityObject> objects_;
    std::vector<CircleItem> items_;
    std::vector<bool> garbage_;
    std::vector<CircleShape> shapes_;
    std::map<std::string, std::unique_ptr<Object>> effects_;
    std::vector<CircleEvent> events_;
    AnimationSet doll_;
    std::optional<size_t> active_;
    std::vector<size_t> strokePoints_;
    std::optional<std::pair<float, float>> previous_;
    bool spellKnown_ = false, entered_ = false, moss_ = false, drawing_ = false, pressing_ = false;
    bool ending_ = false, complete_ = false, strokeAlgorithm_ = false, chalkSound_ = false;
    double age_ = 0, soundAge_ = 0, stopSoundSeconds_ = 0;
    ActivityObject& mutableObject(const std::string& id);
    bool paint(size_t index, float x, float y);
    void drawAt(float x, float y);
    void revealPoint(CircleShape& shape, size_t point, bool expand);
    void updateVisuals();
    void hint(const std::string& key, bool popup = false);
    void checkCleared();
    void checkFinished();
    void sound(const std::string& path);
    void setChalkSound(bool enabled);
};
} // namespace deadtime
