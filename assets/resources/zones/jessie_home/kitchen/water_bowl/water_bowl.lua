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
	addInventoryObject("bowl")
	--gotoState("jessie_home.kitchen.cabinet", "closeused")
end