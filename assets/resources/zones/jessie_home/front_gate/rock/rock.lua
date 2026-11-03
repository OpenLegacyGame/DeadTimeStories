---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("jessie_home.front_gate.fx_blink_rock", true) 
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_stone_move")
end
---clicked
onStateEnter = function (this)
	setTexture(this, "stone_open")
	setCursor(this, "view")
	setInteractive(this, true)
	setVisible("jessie_home.front_gate.fx_blink_rock", true) 
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.rock_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
---used
onStateEnter = function (this)
	setTexture(this, "stone_open")
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("jessie_home.front_gate.fx_blink_rock", false) 
end
