---active
onStateEnter = function (this)
	setDefaultCursor(this, true)
	setInteractive (this, true)
end

---inactive
onStateEnter = function (this)
	setTexture(this, "mailbox_inset_open")
	gotoState("jessie_home.front_gate.mailbox", "activeopen")
end
