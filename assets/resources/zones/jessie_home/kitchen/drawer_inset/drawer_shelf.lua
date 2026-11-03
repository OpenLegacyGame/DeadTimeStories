---active
onStateEnter = function (this)
	--gotoState("jessie_home.kitchen.fx_drawer_shelf", "active") 
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.kitchen.fx_blink_drawer", true)
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.drawer_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
---used
onStateEnter = function (this)
	--gotoState("jessie_home.kitchen.fx_drawer_shelf", "inactive") 
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("jessie_home.kitchen.fx_blink_drawer", false)
end
