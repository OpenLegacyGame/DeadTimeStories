---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setDefaultCursor(this, true)
	setInteractive(this, false)
	setVisible("cajun.swamp_island.fx_blink_sticks", false) 
end

---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("cajun.swamp_island.fx_blink_sticks", true) 
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
	addInventoryObject("sticks")
	setVisible("cajun.swamp_island.fx_blink_sticks", false)
end