dialogText = getString("spellmixing/banish_spirits/congratulations")
dialogImagePath = "/inventory/mixture_banish_spirits";
dialogCloseScript = "addInventoryObject(\"mixture_banish_spirits\")"
showMessageDialogWithImage(dialogText, dialogImagePath, dialogCloseScript)

playSound("/sound/sfx/sfx_item_found")

gotoState("jessie_home.shed.spellmixing", "inactive")