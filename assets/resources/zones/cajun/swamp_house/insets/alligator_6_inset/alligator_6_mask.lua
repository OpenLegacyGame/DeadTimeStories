---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	showHint("%哦 - 那条鳄鱼看起来很悲伤. 它想要什么呢?%")
	playSound("/sound/sfx/sfx_croco_click")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "lollipop" then
		removeInventoryObject("lollipop")
		gotoState(this, "happy")
		gotoState("cajun.alligator_6_inset.inset", "happy")
		stripmineGoodItemAction("lollipop")
	else
		showHint("%那条鳄鱼似乎对它不感兴趣.%")
		stripmineWrongItemAction("lollipop")
	end
end
---happy
onStateEnter = function (this)
	gotoLocation("cajun.swamp_house")
	showHint("%这个幽灵似乎满足了.%")
	gotoState("cajun.swamp_house.alligator_6", "inactive")
	setDefaultCursor (this)
end
