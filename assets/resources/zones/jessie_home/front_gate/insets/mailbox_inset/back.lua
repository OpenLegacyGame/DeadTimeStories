---active
onStateEnter = function (this)
	setCursor(this, "go_down")
end
onLMouseUp = function(this)
	gotoLocation("jessie_home.front_gate")
	playSound("/sound/sfx/sfx_zoom_out")
end