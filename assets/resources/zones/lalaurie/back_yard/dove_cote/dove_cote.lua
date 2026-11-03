---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
	if isMinigameFinishedCellarDoors() then
		gotoState(this, "inactive")
	end
end
onLMouseUp = function (this)
	gotoMinigameDoveCote()
	playSound("/sound/sfx/sfx_zoom_in")
end

---inactive
onStateEnter = function (this)
	setDefaultCursor(this, true)
	setInteractive(this, false)
end