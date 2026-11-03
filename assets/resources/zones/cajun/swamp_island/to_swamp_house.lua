---active
onStateEnter = function (this)
	setCursor(this, "go_down")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	gotoLocation("cajun.swamp_house") 
	playSound("/sound/sfx/sfx_foot_earth")
end