---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.bedroom.fx_blink_saint", true)
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.saint_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
---used
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("jessie_home.bedroom.fx_blink_saint", false)
end
