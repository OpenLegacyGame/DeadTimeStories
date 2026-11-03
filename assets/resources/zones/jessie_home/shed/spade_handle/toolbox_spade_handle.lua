---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
	setDefaultCursor(this)
end
---active
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
	setTexture (this, "toolbox_spade_handle")
end
onLMouseUp = function (this)
	addInventoryObject("spade")
	gotoState(this, "inactive")
	playSound("/sound/sfx/sfx_item_found")
end