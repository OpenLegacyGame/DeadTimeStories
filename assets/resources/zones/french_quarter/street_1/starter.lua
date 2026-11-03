dialogText = getString("dialogs/chapter5/jessie_start")
dialogImage = "/dialogs/talking_face/pt_jessie"
dialogSound = "/sound/dialogues/c5d1_j_thats_mrs_anthons_house"
scriptAfterDialog = ""

showTalkingFaceDialog(dialogText, dialogImage, dialogSound, scriptAfterDialog)

gotoState("french_quarter.apothecary_shop.hob_shelves", "active")
gotoState("french_quarter.morning_room.desk", "active")
gotoState("french_quarter.apothecary_shop.apothecary_shelves", "active")

gotoState("french_quarter.street_2.hint_trigger", "triggered")
gotoState("jessie_home.front_gate.zone_trigger", "c5mid")
