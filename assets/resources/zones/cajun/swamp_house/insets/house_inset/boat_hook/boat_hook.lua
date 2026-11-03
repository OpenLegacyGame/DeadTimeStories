---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setVisible(this, true)
	if wasInventoryObjectFound("boat_hook") then
		gotoState(this, "inactive")
	end
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("boat_hook")
	if wasInventoryObjectFound("blue_anole") and not
		hasInventoryObject("blue_anole") then
		gotoState("cajun.swamp_house.house", "used")
	end
end
---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
end