---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible(this, false)
end
---placed
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible(this, true)
	setVisible("jessie_home.saint_inset.fx_tears", true)
end
---1drop
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible(this, true)
	setTexture(this, "vial1")
end
---2drops
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible(this, true)
	setTexture(this, "vial2")
end
---3drops
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible(this, true)
	setTexture(this, "tears")
end
onLMouseUp = function (this)
	gotoState (this, "clicked")
	gotoState("jessie_home.saint_inset.inset", "inactive")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("tears")
	gotoState("jessie_home.bedroom.saint", "used")
	setVisible("jessie_home.saint_inset.fx_tears", false)
end
