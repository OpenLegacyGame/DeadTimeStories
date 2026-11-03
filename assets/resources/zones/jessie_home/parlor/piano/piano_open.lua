---locked
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
	setDefaultCursor(this)
	setVisible("jessie_home.parlor.fx_flora_piano", false)
end
---unlocked
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.parlor.fx_blink_piano", true)
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.piano_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
---minigame
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setTexture(this, "piano_open")
	setVisible("jessie_home.parlor.fx_flora_piano", true)
	setVisible("jessie_home.parlor.fx_blink_piano", false)
	if isMinigameFinishedPiano() then
		gotoState(this, "inactive")
		gotoState("jessie_home.piano_inset.inset", "opened")
	end
end
onLMouseUp = function (this)
	gotoMinigamePiano()
	playSound("/sound/sfx/sfx_zoom_in")
end
---inactive
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.parlor.fx_flora_piano", false)
	setVisible("jessie_home.parlor.fx_blink_piano", false)
end
onLMouseUp = function (this)
	if not wasInventoryObjectFound("white_pepper") then
		gotoInSet("jessie_home.piano_inset")
		playSound("/sound/sfx/sfx_zoom_in")
	end
	if wasInventoryObjectFound("white_pepper") then
		showHint("%钢琴现在沉默了.%")
	end
end
