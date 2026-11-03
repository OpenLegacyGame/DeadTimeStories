---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
	setDefaultCursor(this, true)
end

---active
onStateEnter = function (this)
	setVisible(this, true)
	gotoState("cajun.mangrove_circle.boat_ready_mask", "active")
end

---clicked
onStateEnter = function (this)
	setVisible(this, true)
end
