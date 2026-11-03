---active
onLMouseUp = function (this)
	gotoState(this, "inactive")
	gotoState("lalaurie.back_yard.barbathioath_piece", "active")
	setAlpha("lalaurie.back_yard.barbathioath_spirit_disappear_fx", 1.0)
	rewindTreeAnimation("lalaurie.back_yard.barbathioath_spirit_disappear_fx", 0.0)
	playTreeAnimation("lalaurie.back_yard.barbathioath_spirit_disappear_fx", false, true)
	playSound("/sound/sfx/mg/reveal_spirits/mg_spirit_free2")
end

---inactive
onStateEnter = function (this)
	setDefaultCursor("lalaurie.back_yard.barbathioath_spirit_mask", true)
	setVisible("lalaurie.back_yard.barbathioath_spirit", false)
	setInteractive(this, false)
end