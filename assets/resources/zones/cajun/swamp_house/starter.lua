dialogText = getString("dialogs/chapter4/mambo_marie_start")
dialogImage = "/dialogs/talking_face/pt_mambo"
dialogSound = "/sound/dialogues/c4d1_mm_i_see_you_there"
scriptAfterDialog = ""

showTalkingFaceDialog(dialogText, dialogImage, dialogSound, scriptAfterDialog)

gotoState("cajun.swamp_island.wringer_corner", "active_1")
gotoState("cajun.mangrove_circle.anole", "active")
gotoState("cajun.mangrove_circle.anole_mask", "preactive")
gotoState("cajun.mangrove_circle.picture", "preactive")
gotoState("jessie_home.front_gate.zone_trigger", "c4mid")
