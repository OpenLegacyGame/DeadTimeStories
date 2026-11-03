---active
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible("jessie_home.front_gate.patch_of_earth_open", false)
	setVisible("jessie_home.front_gate.fx_blink_patch_of_earth", false)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	showHint("%新鲜的泥土... 好像有什么东西埋在这里.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "spade" then
		gotoState(this, "inactive")
		gotoState("jessie_home.front_gate.lantern_glass", "visible")
		setVisible("jessie_home.front_gate.patch_of_earth_open", true)
		setVisible("jessie_home.front_gate.fx_blink_patch_of_earth", false)
		removeInventoryObject("spade")
		stripmineGoodItemAction("spade")
	else
		showHint("%这没用. 我需要找些东西在这里挖掘...%")
		stripmineWrongItemAction("spade")
	end
end
---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("jessie_home.front_gate.patch_of_earth_open", true)
end