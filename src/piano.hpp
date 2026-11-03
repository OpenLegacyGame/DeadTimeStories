#pragma once
#include "activity_layout.hpp"
#include <deque>
#include <map>
#include <optional>

namespace deadtime {
class Piano {
    friend class ActivityCheckpoints;

  public:
    Piano(const Assets& assets, const std::string& path);
    bool press(unsigned key);
    bool takeReward();
    bool skip();
    void tick(double seconds);
    void enter();
    bool busy() const {
        return phase_ == Phase::Demo || phase_ == Phase::NotesFade;
    }
    bool demonstrating() const {
        return phase_ == Phase::Demo;
    }
    bool acceptingNotes() const {
        return phase_ == Phase::Reproduce || phase_ == Phase::ReproduceKey;
    }
    bool opened() const {
        return opened_;
    }
    bool complete() const {
        return collected_;
    }
    size_t round() const {
        return round_;
    }
    size_t progress() const {
        return progress_;
    }
    std::optional<unsigned> soundingKey() const {
        return soundingKey_;
    }
    unsigned minimumKey() const {
        return minimumKey_;
    }
    unsigned maximumKey() const {
        return maximumKey_;
    }
    const auto& rounds() const {
        return rounds_;
    }
    const ActivityObject& key(unsigned index) const;
    const ActivityObject& object(const std::string& name) const;
    const auto& objects() const {
        return objects_;
    }
    std::vector<std::string> takeSounds();
    std::string onEnter, onExit, onFinish, reward;
    double autoplaySeconds = 0;

  private:
    enum class Phase { Idle, Demo, Reproduce, ReproduceKey, NotesFade };
    Phase phase_ = Phase::Idle;
    std::string base_;
    std::vector<ActivityObject> objects_;
    std::vector<std::vector<unsigned>> rounds_;
    std::map<unsigned, double> soundDurations_;
    double errorSeconds_ = 0, remaining_ = 0, openAge_ = 1;
    unsigned minimumKey_ = 0, maximumKey_ = 0;
    size_t round_ = 0, progress_ = 0, demoRound_ = 0, demoNote_ = 0;
    std::optional<unsigned> soundingKey_;
    std::deque<unsigned> queuedKeys_;
    bool opened_ = false, collected_ = false, correctNote_ = false;
    std::vector<std::string> sounds_;
    ActivityObject& mutableObject(const std::string& name);
    std::string keyName(unsigned index) const;
    std::string soundName(unsigned index) const;
    void demo();
    void startNote(unsigned key, bool correct);
    void releaseNote();
    void advance();
    void open();
    void updateVisuals();
};
} // namespace deadtime
