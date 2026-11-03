---active
onStateEnter = function (this)
	setCursor(this, "go_left")
end
onLMouseUp = function (this)
	gotoLocation("french_quarter.apothecary_shop") 
	playSound("/sound/sfx/sfx_foot_concrete")
end