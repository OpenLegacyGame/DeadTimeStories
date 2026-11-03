---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible("jessie_home.parlor.fx_flora_window", false)
end
---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
	--addInventoryObject("mixture_grease_cleaning")
	setVisible("jessie_home.parlor.fx_flora_window", true)
	if isMinigameFinishedWindowCleaning() then
		gotoState(this, "inactive")
	end
end
onLMouseUp = function (this)
	gotoMinigameWindowCleaning()
	playSound("/sound/sfx/sfx_zoom_in")
end
---played
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.parlor.fx_flora_window", false)
end
onLMouseUp = function (this)
	showHint("%日光显得很不正常, 但它至少能让这里看上去欢快一点.%")
end