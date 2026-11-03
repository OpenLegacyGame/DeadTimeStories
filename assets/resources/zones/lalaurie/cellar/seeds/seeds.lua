---active
onStateEnter = function (this)
	--gotoState("lalaurie.cellar.fx_seeds", "active") 
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("lalaurie.cellar.fx_blink_seeds", true) 
end
onLMouseUp = function (this)
	--gotoState("lalaurie.cellar.fx_seeds", "inactive") 
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("seeds")
	setVisible("lalaurie.cellar.fx_blink_seeds", false)
end