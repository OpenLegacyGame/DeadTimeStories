---preactive
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("cajun.mangrove_circle.fx_blink_mango", false)
end 

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
	setVisible("cajun.mangrove_circle.fx_blink_mango", true) 
end
onLMouseUp = function (this)
	showHint("%现在我有了一个芒果, 也许我可以用它来引诱蓝色的变色龙.%")
	playSound("/sound/sfx/sfx_item_use_fail")
	gotoState(this, "inactive")
end

---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	setDefaultCursor(this)
	addInventoryObject("mango")
	playSound("/sound/sfx/sfx_item_found")
	setVisible("cajun.mangrove_circle.fx_blink_mango", false)
end 