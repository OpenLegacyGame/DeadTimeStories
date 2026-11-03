---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("lalaurie.gazebo_exterior.fx_blink_lily", true) 
end
onLMouseUp = function (this)
	showHint("%这是一朵百合花. 我需要用剪子把它剪下来.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end
onInventoryApplied = function (this, inventoryObject)
	playSound("/sound/sfx/sfx_clippers_use")
	if inventoryObject == "clippers" then
		gotoState("lalaurie.gazebo_exterior.lily", "inactive")
		gotoState(this, "inactive")
		addInventoryObject("lily")
		playSound("/sound/sfx/sfx_item_found")
		showHint("%这里... 这可能会有用.%")		
		stripmineGoodItemAction("clippers")
	else
		showHint("%我需要什么东西剪断坚硬的茎.%")
		stripmineWrongItemAction("clippers")
	end
end

---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("lalaurie.gazebo_exterior.fx_blink_lily", false) 
end 
