---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setVisible("jessie_home.kitchen.cabinet", false)
	setVisible("jessie_home.kitchen.fx_blink_cabinet", true)
end
onLMouseUp = function (this)
	gotoState(this, "open")
	playSound("/sound/sfx/sfx_cabinet_open")
end
---open
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible("jessie_home.kitchen.cabinet", true)
	setInteractive(this, false)
	gotoState("jessie_home.kitchen.water_bowl", "active")
	setVisible("jessie_home.kitchen.fx_blink_cabinet", false)
end
onLMouseUp = function (this)
	gotoState(this, "close")
end
---close
onStateEnter = function (this)
	setDefaultCursor(this)
	gotoState("jessie_home.kitchen.water_bowl", "inactive")
	gotoState(this, "active")
	setVisible("jessie_home.kitchen.cabinet", false)
end
---closeused
onStateEnter = function (this)
	setCursor(this, "interact")
	setTexture(this, "cabinet_opened")
	setVisible("jessie_home.kitchen.fx_blink_cabinet", false)
end
onLMouseUp = function (this)
	setDefaultCursor(this)
	gotoState(this, "inactive")
end
---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible("jessie_home.kitchen.cabinet", false)
	setVisible("jessie_home.kitchen.fx_blink_cabinet", false)
end
