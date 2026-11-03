---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("french_quarter.street_2.fx_blink_clock_hands", true) 
end
onLMouseUp = function (this)
	gotoState("french_quarter.street_2.clock_hands", "clicked")
	gotoState(this, "clicked")
	setVisible("french_quarter.street_2.fx_blink_clock_hands", false)
	addInventoryObject("clock_hands")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
end