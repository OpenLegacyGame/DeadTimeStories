---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setDefaultCursor(this, true)
	setInteractive(this, false)
	setVisible("lalaurie.gazebo_exterior.fx_blink_lipstick", false) 
end

---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("lalaurie.gazebo_exterior.fx_blink_lipstick", true) 
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
	addInventoryObject("lipstick")
	setVisible("lalaurie.gazebo_exterior.fx_blink_lipstick", false)
end