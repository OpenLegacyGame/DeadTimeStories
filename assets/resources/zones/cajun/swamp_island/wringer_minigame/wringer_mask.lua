---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function (this)
	gotoMinigameLeeches()
	playSound("/sound/sfx/sfx_zoom_in")
end
---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	gotoState("cajun.swamp_island.drawer", "active") 
end