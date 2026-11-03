---cut
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	setVisible("lalaurie.cellar.fx_blink_cellar_lamp", false)
end
---active
onStateEnter = function (this)
	--gotoState("lalaurie.cellar.fx_cellar_lamp", "active") 
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("lalaurie.cellar.fx_blink_cellar_lamp", true) 
end
onLMouseUp = function (this)
	--gotoState("lalaurie.cellar.fx_cellar_lamp", "inactive") 
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("lamp")
	setVisible("lalaurie.cellar.fx_blink_cellar_lamp", false)
end