#pragma once
#include "activity_layout.hpp"
#include <functional>
#include <map>
#include <optional>
#include <random>
#include <set>

namespace deadtime {
struct MixingIngredient {
    std::string inventory, object, addScript, mixScript, cursorObject;
    float pivotX = 0, pivotY = 0;
    bool fromTable = false, bottle = false;
};
class MixingGesture {
    friend class ActivityCheckpoints;

  public:
    enum class Result { Active, Success, Failure };
    void begin(const std::string& direction, unsigned angle);
    Result move(unsigned angle);

  private:
    unsigned first_ = 0, previous_ = 0;
    bool clockwise_ = true, started_ = false;
    Result result_ = Result::Active;
};
struct MixingRound {
    std::vector<std::string> ingredients, gestures;
};
class SpellMixing {
    friend class ActivityCheckpoints;

  public:
    SpellMixing(const Assets& assets, const std::string& directory, unsigned seed,
                std::string finalInventory = {});
    // The caller checks possession before placing an inventory ingredient.
    bool place(const std::string& inventory);
    bool arranged() const;
    bool putInMortar(const std::string& inventory);
    bool readyToStir() const;
    bool mixtureReady() const {
        return mixtureReady_;
    }
    bool fillBottle();
    bool bottleFilled() const {
        return bottleFilled_;
    }
    bool finish();
    bool skip(const std::function<void(const MixingIngredient&, bool mixed)>& hook);
    std::vector<std::string> skipRequirements;
    bool complete() const {
        return complete_;
    }
    const std::string& finalInventory() const {
        return finalInventory_;
    }
    bool inMortar(const std::string& inventory) const {
        return mortar_.contains(inventory);
    }
    void tick(double seconds);
    bool startStir(unsigned angle);
    MixingGesture::Result moveStir(unsigned angle);
    void releaseStir();
    bool stirring() const {
        return stirring_;
    }
    const std::string& direction() const;
    size_t remainingGestures() const;
    ActivityObject& object(const std::string& name);
    const auto& objects() const {
        return objects_;
    }
    std::string feedback;
    std::optional<size_t> slot(const std::string& inventory) const;
    bool bottlePlaced() const {
        return bottlePlaced_;
    }
    const auto& ingredients() const {
        return ingredients_;
    }
    const auto& rounds() const {
        return rounds_;
    }
    const auto& slots() const {
        return slots_;
    }
    const auto& sceneObjects() const {
        return sceneObjects_;
    }
    std::string onFinish, layout, bottleSlot;
    std::string hintFont;
    uint32_t hintColor = 0;
    float hintClockwiseX = 0, hintClockwiseY = 0, hintCounterclockwiseX = 0, hintCounterclockwiseY = 0;
    bool resetWrongOrder = false, resetWrongMixing = false;
    double firstFailureSeconds = 0, switchedFailureSeconds = 0;
    float mortarLeft = 0, mortarTop = 0, mortarRight = 0, mortarBottom = 0, pestleX = 0, pestleY = 0;

  private:
    std::map<std::string, MixingIngredient> ingredients_;
    std::string finalInventory_;
    std::map<std::string, std::string> sceneObjects_;
    std::vector<std::string> slots_;
    std::vector<MixingRound> rounds_;
    std::map<std::string, size_t> placement_;
    std::mt19937 random_;
    bool bottlePlaced_ = false, mixtureReady_ = false, stirring_ = false, bottleFilled_ = false,
         complete_ = false;
    std::vector<ActivityObject> objects_;
    std::set<std::string> mortar_;
    size_t round_ = 0, gestureIndex_ = 0;
    double mixingAge_ = 0, switchGrace_ = 0;
    MixingGesture gesture_;
    void updateVisuals();
};
} // namespace deadtime
