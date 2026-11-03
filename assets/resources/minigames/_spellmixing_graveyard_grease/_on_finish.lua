dialogText = getString("spellmixing/graveyard_grease/congratulations")
dialogImagePath = "/inventory/mixture_grease_cleaning";
dialogCloseScript = "addInventoryObject(\"mixture_grease_cleaning\")"
showMessageDialogWithImage(dialogText, dialogImagePath, dialogCloseScript)

playSound("/sound/sfx/sfx_item_found")

gotoState("jessie_home.shed.spellmixing", "inactive")