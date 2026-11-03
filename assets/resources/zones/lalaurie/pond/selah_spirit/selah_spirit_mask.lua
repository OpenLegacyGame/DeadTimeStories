---active
onLMouseUp = function (this)
	gotoState(this, "inactive")
	gotoState("lalaurie.pond.selah_piece", "active")
	setAlpha("lalaurie.pond.selah_spirit_disappear_fx", 1.0)
	rewindTreeAnimation("lalaurie.pond.selah_spirit_disappear_fx", 0.0)
	playTreeAnimation("lalaurie.pond.selah_spirit_disappear_fx", false, true)
	playSound("/sound/sfx/mg/reveal_spirits/mg_spirit_free6")
end

---inactive
onStateEnter = function (this)
	setDefaultCursor("lalaurie.pond.selah_spirit_mask", true)
	setVisible("lalaurie.pond.selah_spirit", false)
	setInteractive(this, false)
end