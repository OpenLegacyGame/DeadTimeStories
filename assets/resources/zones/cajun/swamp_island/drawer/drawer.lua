---inactive
onStateEnter = function (this)
	setDefaultCursor(this, true)
	setInteractive (this, false)
end

---active
onStateEnter = function (this)
	setVisible("cajun.swamp_island.fx_blink_drawer", true)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	gotoState(this, "open")
	playSound("/sound/sfx/sfx_drawer_open")
	gotoState("cajun.swamp_island.moonshine", "active")
	gotoState("cajun.swamp_island.thread", "active")
end

---open
onStateEnter = function (this)
	setVisible("cajun.swamp_island.fx_blink_drawer", true)
	setTexture(this, "drawer_open")
	setDefaultCursor(this, true)
	setInteractive (this, false)
	setVisible("cajun.swamp_island.fx_blink_drawer", false)
end