gotoState("jessie_home.shed.spellmixing", "active3")
gotoState("jessie_home.front_gate.zone_trigger", "c3mid")
enableSpell("spirit_revealing_mixture")
gotoState("lalaurie.pond.fishing_basket", "active")
gotoState("lalaurie.back_yard.cellar_door", "active")

dialogText = getString("dialogs/chapter3/jessie_lalaurie_back_yard")
dialogImage = "/dialogs/talking_face/pt_jessie"
dialogSound = "/sound/dialogues/c3d1_j_this_is_the_lalaurie_house"

showTalkingFaceDialog(dialogText, dialogImage, dialogSound)