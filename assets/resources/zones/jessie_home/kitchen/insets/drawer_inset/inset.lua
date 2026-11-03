---active
onStateEnter = function (this)
	setTexture(this, "drawer_closed")
	setCursor(this, "interact")
	gotoState("jessie_home.drawer_inset.wick", "inactive")
end
onLMouseUp = function (this)
	gotoState(this, "open")
	playSound("/sound/sfx/sfx_drawer_open")
end
---open
onStateEnter = function (this)
	setTexture(this, "drawer_open")
	setCursor(this, "interact")
	gotoState("jessie_home.drawer_inset.wick", "active")
end
onLMouseUp = function (this)
	if not hasInventoryObject("wick") then
		gotoState(this, "active")	
		playSound("/sound/sfx/sfx_drawer_close")
	end
	if hasInventoryObject("wick") then
		gotoState(this, "openused")	
		gotoState("jessie_home.kitchen.drawer_shelf", "used")
		playSound("/sound/sfx/sfx_drawer_close")
	end
end
---openused
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
end

