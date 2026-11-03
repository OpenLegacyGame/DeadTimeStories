---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible(this, true)
	setVisible("cemetery.voodoo_shop.fx_flora_hob", true)
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
	addInventoryObject("string")
	addInventoryObject("moss")
	addInventoryObject("chalk")	
	setVisible("cemetery.voodoo_shop.fx_flora_hob", false)
end