---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setTexture(this, "drawer_01_closed")
end
onLMouseUp = function (this)
	gotoState(this, "open")
	playSound("/sound/sfx/sfx_drawer_open")
end
---open
onStateEnter = function (this)
	setDefaultCursor(this)
	setTexture(this, "drawer_01_opened")
	gotoState("jessie_home.shed.salt", "active")
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState(this, "close")
end
---close
onStateEnter = function (this)
	setDefaultCursor(this)
	playSound("/sound/sfx/sfx_drawer_close")
	gotoState("jessie_home.shed.salt", "inactive")
	gotoState(this, "active")
	setTexture(this, "drawer_01_closed")
end
---closeused
onStateEnter = function (this)
	setCursor(this, "interact")
	setTexture(this, "drawer_01_opened")
end
onLMouseUp = function (this)
	setDefaultCursor(this)
	gotoState(this, "inactive")
end
---inactive
onStateEnter = function (this)
	playSound("/sound/sfx/sfx_drawer_close")
	setDefaultCursor(this)
	setInteractive(this, false)
	setTexture(this, "drawer_01_closed")
end
