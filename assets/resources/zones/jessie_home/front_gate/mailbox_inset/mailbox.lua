---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.front_gate.fx_blink_mailbox", true) 
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.mailbox_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
---activeopen
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setTexture(this, "mailbox_opened")
	setVisible("jessie_home.front_gate.fx_blink_mailbox", true) 
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.mailbox_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
---used
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setTexture(this, "mailbox_opened")
	setVisible("jessie_home.front_gate.fx_blink_mailbox", false) 
end
