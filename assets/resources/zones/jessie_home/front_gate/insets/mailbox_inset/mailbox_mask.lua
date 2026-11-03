---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	gotoState("jessie_home.mailbox_inset.brass", "active")
	gotoState("jessie_home.mailbox_inset.inset", "inactive")
	playSound("/sound/sfx/sfx_mailbox_open")
	gotoState(this, "inactive")
end
---inactive
onStateEnter = function (this)
	gotoState("jessie_home.front_gate.mailbox", "activeopen")
	setDefaultCursor (this)
end
