---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact") 
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_skeleton_move")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	gotoState("cajun.swamp_island.pipe", "active")
	setTexture(this, "skeleton_phase_02")
	setInteractive(this, false)
end