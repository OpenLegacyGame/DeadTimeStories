gotoState("jessie_home.parlor.hex_table", "active")

dialogText = getString("dialogs/chapter2/jessie_parlor")
dialogImage = "/dialogs/talking_face/pt_jessie"
dialogSound = "/sound/dialogues/c2d1_j_see_that_mirror"
scriptAfterDialog = 'enableSpell("graveyard_grease")'

showTalkingFaceDialog(dialogText, dialogImage, dialogSound, scriptAfterDialog)