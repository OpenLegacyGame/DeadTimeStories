---active
onStateEnter = function (this)
	--gotoState("lalaurie.back_yard.fx_dirt", "active") 
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("lalaurie.back_yard.fx_blink_dirt", true) 
end
onLMouseUp = function (this)
	--gotoState("lalaurie.back_yard.fx_dirt", "inactive") 
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("dirt")
	setVisible("lalaurie.back_yard.fx_blink_dirt", false)
end