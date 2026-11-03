---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	setDefaultCursor(this, true)
	setVisible("cajun.swamp_island.fx_blink_moonshine", false)
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
	setVisible("cajun.swamp_island.fx_blink_moonshine", true) 
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("moonshine")
	setVisible("cajun.swamp_island.fx_blink_moonshine", false)
end