---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
	setDefaultCursor(this)
end
---active
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setTexture (this, "water_pump_handle")
	setCursor(this, "interact")
	playSound("/sound/sfx/sfx_pump_handle")
end
onLMouseUp = function (this)
	gotoState(this, "used")
	playSound("/sound/sfx/sfx_pump_use")
	if isInState("jessie_home.kitchen.water_pump_jug", "inactive") then
		gotoState("jessie_home.kitchen.fx_water_no_jug", "active")
		showHint("%我需要什么东西来装水.%")
	else
		gotoState("jessie_home.kitchen.fx_water_jug", "active")
	end
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "pump_handle" then
		gotoState(this, "inactive")
		removeInventoryObject("pump_handle")
		stripmineGoodItemAction("pump_handle")
	else
		stripmineWrongItemAction("pump_handle")
	end
	--!
end
---used
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, true)
	setDefaultCursor(this)
end