gotoState("jessie_home.shed.spellmixing", "active5")
gotoState("jessie_home.shed.to_HOB_shed_shelves", "active3")
enableSpell("voodoo_doll")
gotoState("french_quarter.apothecary_shop.hob_shelves", "active_2")
gotoState("french_quarter.morning_room.desk", "active_1")
gotoState("lalaurie.pond.fishing_basket", "active_2")
gotoState("lalaurie.cellar.cellar_corner", "active_1")
gotoState("jessie_home.front_gate.zone_trigger", "c6mid2")

dialogText = getString("dialogs/chapter6/jessie_doll")
dialogImage = "/dialogs/talking_face/pt_jessie"
dialogSound = "/sound/dialogues/c6d2_j_those_childish"
scriptAfterDialog = ''

showTalkingFaceDialog(dialogText, dialogImage, dialogSound, scriptAfterDialog)

