---closed
onStateEnter = function (this)
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
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("mint")
	if hasInventoryObject("clove") then
		gotoState("jessie_home.shed.spice_chest", "used")
	end
end
