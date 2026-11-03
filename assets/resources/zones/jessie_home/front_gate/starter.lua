dialogText = getString("dialogs/chapter1/jessie_start_game")
dialogImage = "/dialogs/talking_face/pt_jessie"
dialogSound = "/sound/dialogues/c1d1_j_im_jessie_bodeen"
scriptAfterDialog = 'startTutorials()'

showTalkingFaceDialog(dialogText, dialogImage, dialogSound, scriptAfterDialog)
showDialog("help_screen")
