---active
onStateEnter = function (this)
	setVisible("jessie_home.front_gate.fx_blink_apple", true) 
	setInteractive(this, true)
	setCursor(this, "interact") 
end
onLMouseUp = function (this)
	setVisible("jessie_home.front_gate.fx_blink_apple", false)
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("apple")
end
