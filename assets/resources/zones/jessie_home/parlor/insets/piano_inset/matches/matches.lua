---active
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	showHint("%我得拿上它. 火柴也许会用的着.%")
	addInventoryObject("matches")
	playSound("/sound/sfx/sfx_item_found")
	
	gotoState(this, "inactive")
	gotoState("jessie_home.parlor.piano_open", "minigame")
	
	gotoState("jessie_home.piano_inset.inset", "opened")
	gotoLocation("jessie_home.parlor")
	gotoMinigamePiano()
end
---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible(this, false)
	
end
