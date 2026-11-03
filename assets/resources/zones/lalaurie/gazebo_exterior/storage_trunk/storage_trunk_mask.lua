---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setVisible("lalaurie.gazebo_exterior.fx_blink_storage_trunk", true) 
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "metal_cutters" then
		gotoState(this, "open")
		gotoState("lalaurie.gazebo_exterior.storage_trunk", "open")
		playSound("/sound/sfx/sfx_padlock_break")
		removeInventoryObject("metal_cutters")
		stripmineGoodItemAction("metal_cutters")
	else
		playSound("/sound/sfx/sfx_item_use_fail")
		showHint("%这没用.%")
		stripmineWrongItemAction("metal_cutters")
	end
end
onLMouseUp = function (this)
	showHint("%箱子被一把挂锁锁着. 也许我可以找到钥匙或是把锁剪下来.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end
---open
onStateEnter = function (this)
	setInteractive(this, false)
	gotoState("lalaurie.gazebo_exterior.saw", "active")
	gotoState("lalaurie.gazebo_exterior.white_paint", "active")
	setDefaultCursor(this) 
	setVisible("lalaurie.gazebo_exterior.fx_blink_storage_trunk", false)
end