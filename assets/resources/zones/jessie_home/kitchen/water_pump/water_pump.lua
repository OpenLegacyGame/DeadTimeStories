---active
onStateEnter = function (this)
	setVisible("jessie_home.kitchen.fx_blink_waper_pump", true)
	setInteractive(this, true)
	setTexture (this, "water_pump")
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	showHint("%这是一个水泵, 但是它的把手不见了.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "pump_handle" then
		gotoState(this, "inactive")
		removeInventoryObject("pump_handle")
		showHint("%我需要什么东西来装水.%")
		stripmineGoodItemAction("pump_handle")
	else
		stripmineWrongItemAction("pump_handle")
	end
end
---inactive
lMousePressed = false
onStateEnter = function (this)
	setInteractive(this, true)
	gotoState("jessie_home.kitchen.water_pump_handle", "active")
	setCursor(this, "interact")
end
onLMouseDown = function (this)
	lMousePressed = true
end
onLMouseUp = function (this)
	if lMousePressed then
		if isInState("jessie_home.kitchen.water_pump_jug", "full") or 
			isInState("jessie_home.kitchen.water_pump_jug", "empty") then
			showHint("%罐子已经放好了%")
		elseif not wasInventoryObjectFound("jug_of_water") then
			showHint("%我需要什么东西来装水.%")
		else
			showHint("%我现在不需要水.%")
		end
		lMousePressed = false
	end
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "empty_jug" then
		gotoState("jessie_home.kitchen.water_pump_jug", "empty")
		removeInventoryObject("empty_jug")
		stripmineGoodItemAction("empty_jug")
	else
		if hasInventoryObject("jug_of_water") then
			showHint("%我现在不需要水.%")
		else
			showHint("%嗯. 我可以找到比这更合适的东西来装水.%")
			stripmineWrongItemAction("empty_jug")
		end
	end
end