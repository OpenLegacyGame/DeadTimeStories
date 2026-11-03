---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	gotoState("jessie_home.music_box_inset.piano_key", "active")
	gotoState(this, "inactive")
	playSound("/sound/sfx/sfx_m_box")
end
---inactive
onStateEnter = function (this)
	setTexture(this, "music_box_inset_open")
	gotoState("jessie_home.bedroom.music_box", "activeopen")
	setDefaultCursor (this)
end
