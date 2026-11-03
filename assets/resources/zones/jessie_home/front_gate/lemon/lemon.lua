---active
onStateEnter = function (this)
	--gotoState("jessie_home.front_gate.fx_lemon", "active") 
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("jessie_home.front_gate.fx_blink_lemon", true) 
end
onLMouseUp = function (this)
	--gotoState("jessie_home.front_gate.fx_lemon", "inactive") 
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("lemon")
	setVisible("jessie_home.front_gate.fx_blink_lemon", false)
end
