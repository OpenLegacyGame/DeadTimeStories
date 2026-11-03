---active
onStateEnter = function (this)
	setCursor(this, "go_right")
end
onLMouseUp = function (this)
	gotoLocation("french_quarter.morning_room") 
	playSound("/sound/sfx/sfx_foot_concrete")
end