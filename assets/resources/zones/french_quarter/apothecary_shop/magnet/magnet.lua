---cut
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	setVisible("french_quarter.apothecary_shop.fx_blink_magnet", false)
end
---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("french_quarter.apothecary_shop.fx_blink_magnet", true) 
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
	addInventoryObject("magnet")
	setVisible("french_quarter.apothecary_shop.fx_blink_magnet", false)
end