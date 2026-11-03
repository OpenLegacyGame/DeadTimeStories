---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
	setDefaultCursor(this)
end
---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "go_up")
end
onLMouseUp = function (this)
	gotoState("jessie_home.front_gate.fx_boards_changing", "inactive")
	gotoLocation("jessie_home.shed") 
	playSound("/sound/sfx/sfx_foot_wood")
end
