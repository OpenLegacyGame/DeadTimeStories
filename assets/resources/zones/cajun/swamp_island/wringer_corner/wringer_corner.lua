---inactive
onStateEnter = function (this)
	setInteractive(this, false)
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("cajun.swamp_island.fx_flora_wringer_corner", true)
end
onLMouseUp = function (this)
	gotoHob("../../../cajun/HOB_wringer_corner.hob", "round0")
	playSound("/sound/sfx/sfx_zoom_in")
end

---active_1
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("cajun.swamp_island.fx_flora_wringer_corner", true)
end
onLMouseUp = function (this)
	gotoHob("../../../cajun/HOB_wringer_corner.hob", "round1")
	playSound("/sound/sfx/sfx_zoom_in")
end

---active_2
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("cajun.swamp_island.fx_flora_wringer_corner", true)
end
onLMouseUp = function (this)
	gotoHob("../../../cajun/HOB_wringer_corner.hob", "round2")
	playSound("/sound/sfx/sfx_zoom_in")
end

---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("cajun.swamp_island.fx_flora_wringer_corner", false)
end
