---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
	setDefaultCursor(this)
	setVisible("jessie_home.kitchen.fx_blink_knob", false)
end
---visible
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
	setVisible("jessie_home.kitchen.fx_blink_knob", true)
end
onLMouseUp = function (this)
	gotoState(this, "inactive")
	addInventoryObject("knob")
	playSound("/sound/sfx/sfx_item_found")
end
