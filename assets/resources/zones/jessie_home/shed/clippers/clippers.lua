---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, false)
end
---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setVisible(this, true)
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("clippers")
	if isPlantBookEnabled() then
	if hasInventoryObject("red_string") then
		if not hasInventoryObject("devils_shoestring") then
				gotoState("jessie_home.front_gate.to_HOB_plants", "active2")
			end
		end
		gotoState("jessie_home.shed.drawer2", "inactive")
	end
end
