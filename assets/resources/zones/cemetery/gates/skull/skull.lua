---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("cemetery.gates.fx_blink_skull", true) 
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
	addInventoryObject("skull")
	setVisible("cemetery.gates.fx_blink_skull", false)
end