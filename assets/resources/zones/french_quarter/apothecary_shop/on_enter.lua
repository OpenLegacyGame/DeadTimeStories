if hasInventoryObject("iodine") and
	hasInventoryObject("morphine") and
	hasInventoryObject("laudanum") and
	isInState("french_quarter.apothecary_shop.pharmacist", "dialog_on") then
	dialogText = getString("dialogs/chapter5/pharmacist1")
	dialogImage = "/dialogs/talking_face/pt_pharmacist"
	dialogSound = "/sound/dialogues/c5d2_ph_a_cuctomer"
	scriptAfterDialog = ""

	showTalkingFaceDialog(dialogText, dialogImage, dialogSound, scriptAfterDialog)
	gotoState("french_quarter.apothecary_shop.pharmacist", "dialog_off")
	setEnabled("french_quarter.apothecary_shop.fx_ghost_man_static", true)
end