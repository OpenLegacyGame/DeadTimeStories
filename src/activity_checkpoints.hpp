#pragma once
#include "apothecary.hpp"
#include "blocked_well.hpp"
#include "cellar_doors.hpp"
#include "cemetery_gate.hpp"
#include "dove_cote.hpp"
#include "hex_bones.hpp"
#include "leeches.hpp"
#include "ouiji.hpp"
#include "piano.hpp"
#include "rats.hpp"
#include "reveal_spirits.hpp"
#include "sail.hpp"
#include "skulls_obey.hpp"
#include "spellmixing.hpp"
#include "spirit_lock.hpp"
#include "voodoo.hpp"
#include "voodoo_circle.hpp"
#include "window_cleaning.hpp"
#include <pugixml.hpp>
namespace deadtime {
class ActivityCheckpoints {
  public:
    static void save(const OuijiBoard& activity, pugi::xml_node node);
    static void load(OuijiBoard& activity, pugi::xml_node node);
    static void save(const SpellMixing& activity, pugi::xml_node node);
    static void load(SpellMixing& activity, pugi::xml_node node);
    static void save(const HexBones& activity, pugi::xml_node node);
    static void load(HexBones& activity, pugi::xml_node node);
    static void save(const Piano& activity, pugi::xml_node node);
    static void load(Piano& activity, pugi::xml_node node);
    static void save(const WindowCleaning& activity, pugi::xml_node node);
    static void load(WindowCleaning& activity, pugi::xml_node node);
    static void save(const CellarDoors& activity, pugi::xml_node node);
    static void load(CellarDoors& activity, pugi::xml_node node);
    static void save(const Rats& activity, pugi::xml_node node);
    static void load(Rats& activity, pugi::xml_node node);
    static void save(const DoveCote& activity, pugi::xml_node node);
    static void load(DoveCote& activity, pugi::xml_node node);
    static void save(const RevealSpirits& activity, pugi::xml_node node);
    static void load(RevealSpirits& activity, pugi::xml_node node);
    static void save(const SpiritLock& activity, pugi::xml_node node);
    static void load(SpiritLock& activity, pugi::xml_node node);
    static void save(const Sail& activity, pugi::xml_node node);
    static void load(Sail& activity, pugi::xml_node node);
    static void save(const Leeches& activity, pugi::xml_node node);
    static void load(Leeches& activity, pugi::xml_node node);
    static void save(const BlockedWell& activity, pugi::xml_node node);
    static void load(BlockedWell& activity, pugi::xml_node node);
    static void save(const Apothecary& activity, pugi::xml_node node);
    static void load(Apothecary& activity, pugi::xml_node node);
    static void save(const CemeteryGate& activity, pugi::xml_node node);
    static void load(CemeteryGate& activity, pugi::xml_node node);
    static void save(const SkullsObey& activity, pugi::xml_node node);
    static void load(SkullsObey& activity, pugi::xml_node node);
    static void save(const VoodooDoll& activity, pugi::xml_node node);
    static void load(VoodooDoll& activity, pugi::xml_node node);
    static void save(const VoodooCircle& activity, pugi::xml_node node);
    static void load(VoodooCircle& activity, pugi::xml_node node);
};
} // namespace deadtime
