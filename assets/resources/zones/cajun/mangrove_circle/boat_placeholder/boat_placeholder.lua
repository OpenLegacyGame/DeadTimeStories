---active
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, true)
end

---solved
onStateEnter = function (this)
	setTexture(this, "boat_w_sail_placeholder")
	gotoState("cajun.mangrove_circle.boat_mask", "solved")
end

---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	gotoState("cajun.mangrove_circle.boat_ready_mask", "active")
end