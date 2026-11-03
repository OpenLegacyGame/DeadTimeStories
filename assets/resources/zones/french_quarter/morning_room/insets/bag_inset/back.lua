---active
onStateEnter = function (this)
	setCursor(this, "go_down")
end
onLMouseUp = function(this)
	gotoLocation("french_quarter.morning_room")
	playSound("/sound/sfx/sfx_zoom_out")
end

