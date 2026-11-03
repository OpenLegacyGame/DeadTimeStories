---active
onStateEnter = function (this)
	setCursor(this, "go_down")
end
onLMouseUp = function(this)
	gotoLocation("french_quarter.street_2")
	playSound("/sound/sfx/sfx_zoom_out")
end

