---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
	setDefaultCursor(this)
end
---visible
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState(this, "inactive")
	addInventoryObject("glass")
	playSound("/sound/sfx/sfx_item_found")
end