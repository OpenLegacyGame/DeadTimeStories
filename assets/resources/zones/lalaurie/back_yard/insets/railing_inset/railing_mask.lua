---safe
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "saw" then
		gotoState(this, "cut")
		gotoState("lalaurie.railing_inset.inset", "cut")
		playSound("/sound/sfx/sfx_saw_use")
		removeInventoryObject("saw")
		stripmineGoodItemAction("saw")
	else
		showHint("%这没用.%")
		playSound("/sound/sfx/sfx_item_use_fail")
		stripmineWrongItemAction("saw")
	end
end
onLMouseUp = function (this)
	showHint("%在这根裂开的木头里面有什么东西, 但是没有工具我不能拿到它.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end

---cut
onStateEnter = function (this)
	setCursor(this, "interact") 
	showHint("%一支画笔! 我得拿上它.%")
	gotoState("lalaurie.back_yard.railing", "cut")
	setVisible("lalaurie.back_yard.fx_blink_railing", false) 
end

onLMouseUp = function (this)
	addInventoryObject("brush")
	playSound("/sound/sfx/sfx_item_found")
	gotoLocation("lalaurie.back_yard")
	gotoState("lalaurie.back_yard.railing", "used")
end	

