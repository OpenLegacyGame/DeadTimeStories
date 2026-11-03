---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	showHint("%炉子里装满了骨灰. 我可以拿一些, 但是我需要什么东西来装.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "bucket" then
		removeInventoryObject("bucket")
		addInventoryObject("ash")
		gotoState(this, "used")
		playSound("/sound/sfx/sfx_item_found")
		showHint("%我可以把骨灰装进水桶里.%")
		stripmineGoodItemAction("bucket")
	else
		showHint("%我不能用这个来装骨灰.%")
		playSound("/sound/sfx/sfx_item_use_fail")
		stripmineWrongItemAction("bucket")
	end
end

---used
onStateEnter = function (this)
	setCursor(this, "interact")
	setVisible("lalaurie.pond.fx_blink_ash", false) 
end
onLMouseUp = function(this)
	showHint("%我不需要更多的骨灰了.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end