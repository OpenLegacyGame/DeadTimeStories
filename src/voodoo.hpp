#pragma once
#include "activity_layout.hpp"
#include <optional>

namespace deadtime {
struct VoodooIngredient {
    std::string inventory, object;
    bool central = false;
    std::vector<std::string> show, hide;
};
class VoodooDoll {
    friend class ActivityCheckpoints;

  public:
    VoodooDoll(const Assets& assets, const std::string& path);
    bool place(const std::string& inventory, bool centralTarget, float x, float y);
    bool apply(const std::string& inventory);
    bool skip();
    bool arranged() const;
    bool complete() const {
        return progress_ == ingredients_.size();
    }
    size_t progress() const {
        return progress_;
    }
    std::optional<size_t> ingredient(const std::string& inventory) const;
    bool used(size_t index) const {
        return index < progress_;
    }
    std::optional<size_t> slot(size_t index) const {
        return placement_.at(index);
    }
    const auto& ingredients() const {
        return ingredients_;
    }
    const auto& objects() const {
        return objects_;
    }
    const auto& slots() const {
        return slots_;
    }
    const ActivityObject& object(const std::string& name) const;
    std::string onFinish, feedback;
    bool rewarded = false;

  private:
    std::vector<VoodooIngredient> ingredients_;
    std::vector<ActivityObject> objects_;
    std::vector<std::string> slots_;
    std::vector<std::optional<size_t>> placement_;
    size_t progress_ = 0;
    ActivityObject& mutableObject(const std::string& name);
    void updateVisuals();
    void advance();
};
} // namespace deadtime
