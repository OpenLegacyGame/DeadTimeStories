---active
onStateEnter = function (this)
	setCursor(this, "go_up")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	gotoLocation("cajun.mangrove_circle") 
	playSound("/sound/sfx/sfx_foot_earth")
end