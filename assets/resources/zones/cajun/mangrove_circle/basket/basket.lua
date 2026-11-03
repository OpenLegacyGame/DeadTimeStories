---invisible
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible("cajun.mangrove_circle.fx_flora_basket", false)
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("cajun.mangrove_circle.fx_flora_basket", true)
end
onLMouseUp = function (this)
	gotoHob("../../../cajun/HOB_basket.hob", "round0")
end

---active_1
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("cajun.mangrove_circle.fx_flora_basket", true)
end
onLMouseUp = function (this)
	gotoHob("../../../cajun/HOB_basket.hob", "round1")
end

---active_2
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("cajun.mangrove_circle.fx_flora_basket", true)
end
onLMouseUp = function (this)
	gotoHob("../../../cajun/HOB_basket.hob", "round2")
end

---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("cajun.mangrove_circle.fx_flora_basket", false)
end
