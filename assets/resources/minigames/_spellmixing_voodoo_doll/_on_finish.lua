prompt = getString("spellmixing/voodoo_doll/congratulations")
image = "/inventory/voodoo_doll"
scriptOnClose =
	'addInventoryObject("voodoo_doll")\n'..
	'exitMinigame()'
showMessageDialogWithImage(prompt, image, scriptOnClose)

gotoState("jessie_home.shed.spellmixing", "inactive")