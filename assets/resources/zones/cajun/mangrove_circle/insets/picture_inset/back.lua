---active
onStateEnter = function (this)
	setCursor(this, "go_down")
end
onLMouseUp = function(this)
	gotoLocation("cajun.mangrove_circle")
	playSound("/sound/sfx/sfx_zoom_out")
end

