---active
onLMouseUp = function (this)
	gotoState(this, "inactive")
	gotoState("lalaurie.pond.matba_piece", "active")
	setAlpha("lalaurie.pond.matba_spirit_disappear_fx", 1.0)
	rewindTreeAnimation("lalaurie.pond.matba_spirit_disappear_fx", 0.0)
	playTreeAnimation("lalaurie.pond.matba_spirit_disappear_fx", false, true)
	playSound("/sound/sfx/mg/reveal_spirits/mg_spirit_free1")
end

---inactive
onStateEnter = function (this)
	setDefaultCursor("lalaurie.pond.matba_spirit_mask", true)
	setVisible("lalaurie.pond.matba_spirit", false)
	setInteractive(this, false)
end