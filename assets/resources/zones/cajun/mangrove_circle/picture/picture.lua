---start
onStateEnter = function (this)
	setInteractive(this, true)
end

---preactive
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible(this, true)
	setVisible("cajun.mangrove_circle.fx_blink_picture", true)
end
onLMouseUp = function (this)
	gotoInSet("cajun.picture_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible(this, true)
	setVisible("cajun.mangrove_circle.fx_blink_picture", true)
end
onLMouseUp = function (this)
	gotoInSet("cajun.picture_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	setVisible("cajun.mangrove_circle.fx_blink_picture", false)
end