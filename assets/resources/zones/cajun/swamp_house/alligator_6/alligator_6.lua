---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("cajun.swamp_house.fx_blink_alligator_6", true)
end
onLMouseUp = function (this)
	gotoInSet("cajun.alligator_6_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
---inactive
onStateEnter = function (this)
	playSound("/sound/sfx/sfx_croco_gone")
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible(this, false)
	setVisible("cajun.swamp_house.fx_blink_alligator_6", false)
	if isInState("cajun.swamp_house.alligator_2", "inactive") and
		isInState("cajun.swamp_house.alligator_3", "inactive") and
		isInState("cajun.swamp_house.alligator_4", "inactive") and
		isInState("cajun.swamp_house.alligator_5", "inactive") and
		isInState("cajun.swamp_house.alligator_1", "inactive") then
		gotoState("cajun.swamp_house.house", "open")
		showMessageDialog("%行了, 现在我可以到门廊上去了.%")
	end
end
