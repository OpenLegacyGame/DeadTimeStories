---active
onLMouseUp = function (this)
	gotoState(this, "inactive")
	gotoState("lalaurie.pond.harrahya_piece", "active")
	setAlpha("lalaurie.pond.harrahya_spirit_disappear_fx", 1.0)
	rewindTreeAnimation("lalaurie.pond.harrahya_spirit_disappear_fx", 0.0)
	playTreeAnimation("lalaurie.pond.harrahya_spirit_disappear_fx", false, true)
	playSound("/sound/sfx/mg/reveal_spirits/mg_spirit_free2")
end

---inactive
onStateEnter = function (this)
	setDefaultCursor("lalaurie.pond.harrahya_spirit_mask", true)
	setVisible("lalaurie.pond.harrahya_spirit", false)
	setInteractive(this, false)
end