---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
end

onLMouseUp = function (this)
	if not hasInventoryObject("bloomers") and
		not hasInventoryObject("nightdress") and
		not hasInventoryObject("shirt") and
		not hasInventoryObject("tablecloth") and
		not hasInventoryObject("trowel") then
		showHint("%这条小船缺了一根桨.%")
	end
	gotoMinigameSail()
	playSound("/sound/sfx/sfx_zoom_in")
end

---solved
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState("cajun.mangrove_circle.boat_ready", "active")
	gotoState("cajun.mangrove_circle.boat_ready_mask", "active")
	gotoState(this, "clicked")
	gotoState("cajun.mangrove_circle.boat_placeholder", "clicked")
	playSound("/sound/sfx/sfx_water_splash1")
end


---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setVisible("cajun.mangrove_circle.fx_candles", false)
	setVisible("cajun.mangrove_circle.fx_candles_r", true)
	setInteractive(this, false)
	gotoState("cajun.mangrove_circle.basket", "active")
end