---active
onStateEnter = function (this)
	setCursor(this, "go_down")
end
onLMouseUp = function(this)
	gotoLocation("french_quarter.street_1")
	playSound("/sound/sfx/sfx_zoom_out")
end

