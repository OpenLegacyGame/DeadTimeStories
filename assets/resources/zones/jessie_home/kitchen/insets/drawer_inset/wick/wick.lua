---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
end
---active
onStateEnter = function (this)
	--gotoState("jessie_home.drawer_inset.fx_wick", "active") 
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	--gotoState("jessie_home.drawer_inset.fx_wick", "inactive") 
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("wick")
	gotoState("jessie_home.kitchen.drawer_shelf", "used")
	gotoState("jessie_home.drawer_inset.inset", "openused")
end
