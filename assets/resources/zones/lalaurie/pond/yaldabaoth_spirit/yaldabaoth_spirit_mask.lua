---active
onLMouseUp = function (this)
	gotoState(this, "inactive")
	gotoState("lalaurie.pond.yaldabaoth_piece", "active")
	setAlpha("lalaurie.pond.yaldabaoth_spirit_disappear_fx", 1.0)
	rewindTreeAnimation("lalaurie.pond.yaldabaoth_spirit_disappear_fx", 0.0)
	playTreeAnimation("lalaurie.pond.yaldabaoth_spirit_disappear_fx", false, true)
	playSound("/sound/sfx/mg/reveal_spirits/mg_spirit_free5")
end

---inactive
onStateEnter = function (this)
	setDefaultCursor("lalaurie.pond.yaldabaoth_spirit_mask", true)
	setVisible("lalaurie.pond.yaldabaoth_spirit", false)
	setInteractive(this, false)
end