#pragma once
#include "runtime.hpp"
namespace deadtime {
// The menu's native scene is separate from world locations and Lua state.
class MenuScene {
  public:
    explicit MenuScene(const Assets& assets);
    void hover(const std::string& button);
    Scene& scene() {
        return scene_;
    }

  private:
    Scene scene_;
    std::vector<std::unique_ptr<Object>> objects_;
};
} // namespace deadtime
