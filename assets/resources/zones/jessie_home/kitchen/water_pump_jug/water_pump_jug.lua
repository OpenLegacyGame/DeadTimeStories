---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
	setDefaultCursor(this)
end
---full
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
	setTexture (this, "water_pump_jug_full")
	gotoState("jessie_home.kitchen.water_pump_handle", "used")
end
onLMouseUp = function (this)
	gotoState(this, "used")
	addInventoryObject("jug_of_water")
	playSound("/sound/sfx/sfx_item_found")
end
---empty
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, true)
	setDefaultCursor(this)
	setTexture (this, "water_pump_jug_empty")
end
---used
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
	setDefaultCursor(this)
end