---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)	
	setDefaultCursor(this, true)
	setVisible("lalaurie.gazebo_exterior.fx_blink_white_paint", false)	
end

---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setVisible(this, true)
	setVisible("lalaurie.gazebo_exterior.fx_blink_white_paint", true)	
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("white_paint")
	setVisible("lalaurie.gazebo_exterior.fx_blink_white_paint", false)	
end
