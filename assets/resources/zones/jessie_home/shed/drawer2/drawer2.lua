---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setTexture(this, "drawer_02_closed")
end
onLMouseUp = function (this)
	gotoState(this, "open")
	playSound("/sound/sfx/sfx_drawer_open")
end
---open
onStateEnter = function (this)
	setDefaultCursor(this)
	setTexture(this, "drawer_02_opened")
	if not hasInventoryObject("clippers") then
			gotoState("jessie_home.shed.clippers", "active")
		end
	if not isPlantBookEnabled() then
			gotoState("jessie_home.shed.plant_book", "active")
	end
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState(this, "close")
	playSound("/sound/sfx/sfx_drawer_close")
end
---close
onStateEnter = function (this)
	setDefaultCursor(this)
	gotoState("jessie_home.shed.clippers", "inactive")
	gotoState("jessie_home.shed.plant_book", "inactive")
	gotoState(this, "active")
	setTexture(this, "drawer_02_closed")
end
---closeused
onStateEnter = function (this)
	setCursor(this, "interact")
	setTexture(this, "drawer_02_opened")
end
onLMouseUp = function (this)
	setDefaultCursor(this)
	gotoState(this, "inactive")
	playSound("/sound/sfx/sfx_drawer_close")
end
---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setTexture(this, "drawer_02_closed")
end

