dialogText = getString("spellmixing/aconite_antidote/congratulations")
dialogImagePath = "/inventory/antidote";
dialogCloseScript = 'addInventoryObject("antidote")'
showMessageDialogWithImage(dialogText, dialogImagePath, dialogCloseScript)

playSound("/sound/sfx/sfx_item_found")

gotoState("jessie_home.shed.spellmixing", "inactive")
