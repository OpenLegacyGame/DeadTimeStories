---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	showHint("%那一条看起来骨瘦如柴. 我最好让它离开 - 但是怎么做呢?%")
	playSound("/sound/sfx/sfx_croco_click")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "pipe" then
		removeInventoryObject("pipe")
		gotoState(this, "happy")
		gotoState("cajun.alligator_5_inset.inset", "happy")
		stripmineGoodItemAction("pipe")
	else
		showHint("%那条鳄鱼似乎对它不感兴趣.%")
		stripmineWrongItemAction("pipe")
	end
end
---happy
onStateEnter = function (this)
	gotoLocation("cajun.swamp_house")
	showHint("%这个幽灵似乎满足了.%")
	gotoState("cajun.swamp_house.alligator_5", "inactive")
	setDefaultCursor (this)
end
