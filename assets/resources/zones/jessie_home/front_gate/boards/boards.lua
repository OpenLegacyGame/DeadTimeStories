---active
onStateEnter = function (this)
	setVisible("jessie_home.front_gate.fx_blink_boards", true)
	setInteractive(this, true)
	setCursor(this, "interact") 
end
onLMouseUp = function (this)
	setVisible("jessie_home.front_gate.fx_blink_boards", false)
	setVisible("jessie_home.front_gate.boards_anim", false)
	playSound("/sound/sfx/sfx_board_move")
	gotoState(this, "clicked")
	gotoState("jessie_home.front_gate.fx_boards_changing", "active")
end
---clicked
onStateEnter = function (this)
	setAlpha(this, 1)
	setTexture(this, "boards_open")
	setInteractive(this, false)
	gotoState("jessie_home.front_gate.to_shed", "active")
end
