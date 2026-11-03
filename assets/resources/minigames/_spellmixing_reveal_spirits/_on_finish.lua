dialogText = getString("spellmixing/spirit_revealing_mixture/congratulations")
dialogImagePath = "/inventory/spirit_revealing_mixture";
dialogCloseScript = 'addInventoryObject("spirit_revealing_mixture")'
showMessageDialogWithImage(dialogText, dialogImagePath, dialogCloseScript)

playSound("/sound/sfx/sfx_item_found")

gotoState("jessie_home.shed.spellmixing", "inactive")

addAnywhereAppliable("lalaurie.back_yard", "spirit_revealing_mixture", "/zones/lalaurie/back_yard/reveal_spell/on_apply_mixture")
addAnywhereAppliable("lalaurie.cellar", "spirit_revealing_mixture", "/zones/lalaurie/cellar/reveal_spell/on_apply_mixture")
addAnywhereAppliable("lalaurie.gazebo_exterior", "spirit_revealing_mixture", "/zones/lalaurie/gazebo_exterior/reveal_spell/on_apply_mixture")
addAnywhereAppliable("lalaurie.pond", "spirit_revealing_mixture", "/zones/lalaurie/pond/reveal_spell/on_apply_mixture")
