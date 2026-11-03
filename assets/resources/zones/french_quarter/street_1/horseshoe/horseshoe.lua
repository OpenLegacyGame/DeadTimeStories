---active
onStateEnter = function (this)
	--gotoState("french_quarter.street_1.fx_horseshoe", "active") 
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("french_quarter.street_1.fx_blink_horseshoe", true) 
end
onLMouseUp = function (this)
	--gotoState("french_quarter.street_1.fx_horseshoe", "inactive") 
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("horseshoe")
	setVisible("french_quarter.street_1.fx_blink_horseshoe", false)
end