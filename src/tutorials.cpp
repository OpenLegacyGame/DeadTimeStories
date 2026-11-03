#include "tutorials.hpp"
#include <array>
namespace deadtime {
const TutorialSpec& tutorialSpec(TutorialId id) {
    // Native 8CF860 table, indexed by the original HintBubble enum.
    static constexpr std::array<TutorialSpec, 16> specs{{{"first_item_found", 211, 711},
                                                         {"inventory_picture", 185, 581, false, true},
                                                         {"second_item_found1", 500, 730},
                                                         {"second_item_found2", 207, 657},
                                                         {"first_hob", 530, 670},
                                                         {"hob_10sec", 970, 670},
                                                         {"hob_plant", 916, 643},
                                                         {"location_hint", 970, 670},
                                                         {"show_hint", 500, 100},
                                                         {"ouija", 970, 670},
                                                         {"spell_book", 664, 161},
                                                         {"spell_mix", 640, 470},
                                                         {"carriage", 40, 470},
                                                         {"new_areas", 40, 470, true},
                                                         {"sail_piece_move_rotate", 0, 0},
                                                         {"new_spell_added", 110, 628, true}}};
    return specs.at(static_cast<size_t>(id));
}
} // namespace deadtime
