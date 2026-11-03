---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	setVisible("jessie_home.kitchen.fx_blink_garlic_head", false)
end
---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("jessie_home.kitchen.fx_blink_garlic_head", true)
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	addInventoryObject("garlic_head")
	playSound("/sound/sfx/sfx_item_found")
end