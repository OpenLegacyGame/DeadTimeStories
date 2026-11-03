---inactive
onStateEnter = function (this)
	setInteractive(this, false)
end

---preactive
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("cajun.mangrove_circle.fx_blink_anole", true) 
end
onLMouseUp = function (this)
	gotoState(this, "hidden")
	gotoState("cajun.mangrove_circle.anole", "hidden")
	showHint("%嗨! 等等! 我猜这个小家伙很害羞. 也许我可以用什么东西把它引出来.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "mango" then
		removeInventoryObject("mango")
		gotoState(this, "eating")
		gotoState("cajun.mangrove_circle.anole", "eating")
		stripmineGoodItemAction("mango")
	else
		showHint("%不, 它对这个不感兴趣.%")
		stripmineWrongItemAction("mango")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("cajun.mangrove_circle.fx_blink_anole", true) 
end
onLMouseUp = function (this)
	gotoState(this, "hidden")
	gotoState("cajun.mangrove_circle.anole", "hidden")
	showHint("%嗨! 等等! 我猜这个小家伙很害羞. 也许我可以用什么东西把它引出来.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "mango" then
		removeInventoryObject("mango")
		gotoState(this, "eating")
		gotoState("cajun.mangrove_circle.anole", "eating")
		stripmineGoodItemAction("mango")
		playSound("/sound/sfx/sfx_inv_item_put")
	else
		showHint("%不, 它对这个不感兴趣.%")
		stripmineWrongItemAction("mango")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end

---hidden
onStateEnter = function (this)
	playSound("/sound/sfx/sfx_anole_hiding")
	setInteractive(this, false)
	setTimer(this, 1)
end

onTimer = function(this)
	gotoState(this, "active")	
	gotoState("cajun.mangrove_circle.anole", "active")	
end

---eating
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("cajun.mangrove_circle.fx_blink_anole", true) 
	showHint("%嗨, 小家伙... 想要一个漂亮的, 熟透的芒果吗?%")
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	gotoState("cajun.mangrove_circle.anole", "clicked")
	showHint("%我捉到它了!%")
	playSound("/sound/sfx/sfx_item_found")
end

---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	addInventoryObject("blue_anole")
	setInteractive(this, false)
	setVisible("cajun.mangrove_circle.fx_blink_anole", false) 
end