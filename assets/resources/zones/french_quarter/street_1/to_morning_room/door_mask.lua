---preactive
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
	setVisible("french_quarter.street_1.fx_blink_antons_door", true) 
end
onLMouseUp = function(this)
	gotoState(this, "active")
	showHint("%我很想进去, 但是门锁着. 我需要钥匙.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "antons_key" then
		removeInventoryObject("antons_key")
		gotoState(this, "opened")
		stripmineGoodItemAction("antons_key")
	else
		showHint("%那不是安东夫人家的钥匙.%")
		stripmineWrongItemAction("antons_key")
	end
end

---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
	setVisible("french_quarter.street_1.fx_blink_antons_door", true) 
end
onLMouseUp = function(this)
	showHint("%我很想进去, 但是门锁着. 我需要钥匙.%")
	playSound("/sound/sfx/sfx_drawer_locked")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "antons_key" then
		removeInventoryObject("antons_key")
		gotoState(this, "opened")
		stripmineGoodItemAction("antons_key")
		playSound("/sound/sfx/sfx_key_use")
	else
		showHint("%那不是安东夫人家的钥匙.%")
		stripmineWrongItemAction("antons_key")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end


---opened
onStateEnter = function (this)
	setVisible("french_quarter.street_1.antons_door", true)
	setCursor(this, "go_right")
	setInteractive(this, true)
	setVisible("french_quarter.street_1.fx_blink_antons_door", false) 
end
onLMouseUp = function(this)
	gotoLocation("french_quarter.morning_room") 
	playSound("/sound/sfx/sfx_foot_concrete")
end