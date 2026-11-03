---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("french_quarter.apothecary_shop.fx_blink_binoculars", true) 
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
	addInventoryObject("binoculars")
	setVisible("french_quarter.apothecary_shop.fx_blink_binoculars", false)
end