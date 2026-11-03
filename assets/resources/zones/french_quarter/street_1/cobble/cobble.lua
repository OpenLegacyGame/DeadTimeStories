---active
onStateEnter = function (this)
	--gotoState("french_quarter.street_1.fx_cobble", "active") 
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("french_quarter.street_1.fx_blink_cobble", true) 
end
onLMouseUp = function (this)
	--gotoState("french_quarter.street_1.fx_cobble", "inactive") 
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("cobble")
	setVisible("french_quarter.street_1.fx_blink_cobble", false)
end