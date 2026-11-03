---active
onStateEnter = function (this)
	setCursor(this, "go_down")
end
onLMouseUp = function(this)
	gotoLocation("jessie_home.front_gate")
	playSound("/sound/sfx/sfx_zoom_out")
end
---inactive
onLMouseUp = function(this)
	gotoLocation("jessie_home.front_gate")
	gotoState("jessie_home.front_gate.fx_rock_inset", "inactive")
end
