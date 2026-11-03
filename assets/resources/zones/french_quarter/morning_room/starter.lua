dialogText = getString("dialogs/chapter5/doctor")
dialogImage = "/dialogs/talking_face/pt_doctor"
dialogSound = "/sound/dialogues/c5d4_dr_i_must_have_something"
scriptAfterDialog = ''

showTalkingFaceDialog(dialogText, dialogImage, dialogSound, scriptAfterDialog)

gotoState("french_quarter.morning_room.bag", "active")
