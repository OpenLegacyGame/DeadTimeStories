---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
end
---active
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState("jessie_home.shed.snake", "active")
	gotoState("jessie_home.snake_inset.inset", "active")
	gotoLocation("jessie_home.shed")
end
