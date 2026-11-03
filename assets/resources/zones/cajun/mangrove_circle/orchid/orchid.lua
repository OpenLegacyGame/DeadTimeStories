---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("cajun.mangrove_circle.fx_blink_orchid", true) 
end
onLMouseUp = function (this)
	showHint("%这是一株兰花. 我可以用剪子把它剪下来.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end

onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "clippers" then
		gotoState(this, "inactive")
		playSound("/sound/sfx/sfx_clippers_use")
		showHint("%这里... 这可能会有用.%")		
		stripmineGoodItemAction("clippers")
	else
		showHint("%我需要什么东西把花剪下来.%")
		stripmineWrongItemAction("clippers")
	end
end

---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	setDefaultCursor(this)
	addInventoryObject("orchid")
	playSound("/sound/sfx/sfx_item_found")
	setVisible("cajun.mangrove_circle.fx_blink_orchid", false)
end 