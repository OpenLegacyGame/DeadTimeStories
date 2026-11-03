---inactive
onStateEnter = function (this)
	-- gotoState("jessie_home.mailbox_inset.fx_brass", "inactive") 
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible(this, false)
end
---active
onStateEnter = function (this)
	--gotoState("jessie_home.mailbox_inset.fx_brass", "active") 
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	--gotoState("jessie_home.mailbox_inset.fx_brass", "inactive") 
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("brass")
	gotoState("jessie_home.front_gate.mailbox", "used")
end
