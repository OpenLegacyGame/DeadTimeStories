---active
onStateEnter = function (this)
	setCursor(this, "go_down")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	gotoLocation("cemetery.gates") 
	playSound("/sound/sfx/sfx_foot_concrete")
end