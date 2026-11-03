---inactive
onStateEnter = function (this)
	setInteractive(this, false)
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("french_quarter.morning_room.fx_flora_desk", true)
end
onLMouseUp = function (this)
	gotoHob("../../../french_quarter/HOB_desk.hob", "round0")
	playSound("/sound/sfx/sfx_zoom_in")
end

---active_1
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("french_quarter.morning_room.fx_flora_desk", true)
end
onLMouseUp = function (this)
	gotoHob("../../../french_quarter/HOB_desk.hob", "round1")
	playSound("/sound/sfx/sfx_zoom_in")
end

---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("french_quarter.morning_room.fx_flora_desk", false)
end