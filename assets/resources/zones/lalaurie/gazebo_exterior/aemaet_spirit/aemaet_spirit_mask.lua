---active
onLMouseUp = function (this)
	gotoState(this, "inactive")
	gotoState("lalaurie.gazebo_exterior.aemaet_piece", "active")
	setAlpha("lalaurie.gazebo_exterior.aemaet_spirit_disappear_fx", 1.0)
	rewindTreeAnimation("lalaurie.gazebo_exterior.aemaet_spirit_disappear_fx", 0.0)
	playTreeAnimation("lalaurie.gazebo_exterior.aemaet_spirit_disappear_fx", false, true)
	playSound("/sound/sfx/mg/reveal_spirits/mg_spirit_free3")
end

---inactive
onStateEnter = function (this)
	setDefaultCursor("lalaurie.gazebo_exterior.aemaet_spirit_mask", true)
	setVisible("lalaurie.gazebo_exterior.aemaet_spirit", false)
	setInteractive(this, false)
end