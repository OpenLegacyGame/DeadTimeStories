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
	addInventoryObject("antons_key")
	gotoState("french_quarter.street_2.well", "clicked")
end
