---active
onStateEnter = function (this)
	setTexture(this, "drawer_mask")
	setCursor(this, "interact")
	setVisible("jessie_home.openme_inset.fx_smoke", false)
	setVisible("jessie_home.openme_inset.fx_blink_drawer", true)
end
onLMouseUp = function (this)
	gotoState(this, "open")
	gotoState("jessie_home.openme_inset.inset", "open")
	playSound("/sound/sfx/sfx_drawer_open")
	setVisible("jessie_home.openme_inset.fx_blink_drawer", false)
	showHint("%在底部好像有什么东西烧成了这些字!%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "key" then
		removeInventoryObject("key")
		gotoLocation("jessie_home.shed")
		gotoState("jessie_home.shed.drawer4", "open")
		stripmineGoodItemAction("key")
	else
		showHint("%这不是钥匙.%")
		stripmineWrongItemAction("key")
	end
end
---open
onStateEnter = function (this)
	setCursor(this, "interact")
	setVisible("jessie_home.openme_inset.fx_smoke", true)
end
onLMouseUp = function (this)
	gotoState(this, "openused")
	gotoState("jessie_home.openme_inset.inset", "openused")
end
---openused
onStateEnter = function (this)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState(this, "active")	
	gotoState("jessie_home.openme_inset.inset", "active")
	playSound("/sound/sfx/sfx_drawer_close")
end
