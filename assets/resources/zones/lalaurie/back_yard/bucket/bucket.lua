---active
onStateEnter = function (this)
	--gotoState("lalaurie.back_yard.fx_bucket", "active") 
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("lalaurie.back_yard.fx_blink_bucket", true) 
end
onLMouseUp = function (this)
	--gotoState("lalaurie.back_yard.fx_bucket", "inactive") 
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("bucket")
	setVisible("lalaurie.back_yard.fx_blink_bucket", false)
end