---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setVisible(this, true)
end
onLMouseUp = function (this)
	showHint("%我应该把蓝变色龙放在这里!%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "blue_anole" then
		gotoState(this, "clicked")
		playSound("/sound/sfx/mg/mg_finish")
		gotoCutscene("ch4_end", "/zones/cajun/swamp_house/cs_finish", true)
--		allowZoneAccess("french_quarter")
		removeInventoryObject("blue_anole")
		stripmineGoodItemAction("blue_anole")
		gotoState("jessie_home.front_gate.zone_trigger", "c4end")
	else
		showHint("%这不是蓝变色龙.%")
		stripmineWrongItemAction("blue_anole")
	end 
end

---clicked
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	if wasInventoryObjectFound("boat_hook") then
		gotoState("cajun.swamp_house.house", "used")
	end
end
