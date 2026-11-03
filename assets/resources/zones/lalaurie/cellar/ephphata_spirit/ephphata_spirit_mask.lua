---active
onLMouseUp = function (this)
	gotoState(this, "inactive")
	gotoState("lalaurie.cellar.ephphata_piece", "active")
	setAlpha("lalaurie.cellar.ephphata_spirit_disappear_fx", 1.0)
	rewindTreeAnimation("lalaurie.cellar.ephphata_spirit_disappear_fx", 0.0)
	playTreeAnimation("lalaurie.cellar.ephphata_spirit_disappear_fx", false, true)
	playSound("/sound/sfx/mg/reveal_spirits/mg_spirit_free4")
end

---inactive
onStateEnter = function (this)
	setDefaultCursor("lalaurie.cellar.ephphata_spirit_mask", true)
	setVisible("lalaurie.cellar.ephphata_spirit", false)
	setInteractive(this, false)
end