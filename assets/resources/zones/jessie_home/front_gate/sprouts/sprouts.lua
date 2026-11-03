---active
onStateEnter = function (this)
	setInteractive(this, true)
	setTexture (this, "sprouts")
	setCursor(this, "interact")
	setVisible("jessie_home.front_gate.fx_blink_sprouts", true) 
end
onLMouseUp = function (this)
	showHint("%这里生长着什么东西. 如果我在泥土上浇点水, 也许会有植物长出来.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "watering_can" then
		gotoState(this, "watered")
		gotoState("jessie_home.front_gate.fx_sprouts_changing", "active")
		removeInventoryObject("watering_can")
		playSound("/sound/sfx/sfx_watercan_use")
		stripmineGoodItemAction("watering_can")
	else
		showHint("%这里生长着什么东西. 如果我在泥土上浇点水, 也许会有植物长出来.%")
		stripmineWrongItemAction("watering_can")
	end
end
---inactive
onStateEnter = function (this)
	setTexture (this, "sprouts_used")
	setInteractive(this, false)
	gotoState("jessie_home.front_gate.sprout1", "inactive")
	gotoState("jessie_home.front_gate.sprout3", "inactive")
	gotoState("jessie_home.front_gate.sprout2", "inactive")
	setDefaultCursor(this)
	setVisible("jessie_home.front_gate.fx_blink_sprouts", false) 
end
---watered
onStateEnter = function (this)
	setTexture (this, "sprouts_used")
	setInteractive(this, false)
	gotoState("jessie_home.front_gate.sprout1", "active")
	gotoState("jessie_home.front_gate.sprout2", "active")
	gotoState("jessie_home.front_gate.sprout3", "active")
	setDefaultCursor(this)
	setVisible("jessie_home.front_gate.fx_blink_sprouts", true) 
end
