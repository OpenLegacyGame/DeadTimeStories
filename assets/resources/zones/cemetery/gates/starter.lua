gotoState("cajun.swamp_island.wringer_corner", "active_2")
gotoState("cajun.mangrove_circle.basket", "active_2")
gotoState("cemetery.gates.hint_trigger", "triggered")
gotoState("jessie_home.front_gate.zone_trigger", "c6mid1")

dialogText = getString("dialogs/chapter6/jessie_shop")
dialogImage = "/dialogs/talking_face/pt_jessie"
dialogSound = "/sound/dialogues/c6d1_j_i_was_so_angry"
scriptAfterDialog = ''

showTalkingFaceDialog(dialogText, dialogImage, dialogSound, scriptAfterDialog)



