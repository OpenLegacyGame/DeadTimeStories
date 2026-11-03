---active
onStateEnter = function (this)
	setCursor(this, "go_up")
end
onLMouseUp = function (this)
	gotoLocation("french_quarter.street_2") 
	playSound("/sound/sfx/sfx_foot_concrete")
end