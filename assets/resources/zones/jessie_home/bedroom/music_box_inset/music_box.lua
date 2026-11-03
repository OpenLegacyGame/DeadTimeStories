---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.bedroom.fx_blink_music_box", true)
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.music_box_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
---activeopen
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setTexture(this, "music_box_opened")
	setVisible("jessie_home.bedroom.fx_blink_music_box", true)
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.music_box_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
---used
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setTexture(this, "music_box_opened")
	setVisible("jessie_home.bedroom.fx_blink_music_box", false)
end
