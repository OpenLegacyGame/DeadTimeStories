---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	showHint("%那条鳄鱼的头上顶着什么东西. 好像是顶帽子!%")
	playSound("/sound/sfx/sfx_croco_click")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "orchid" then
		removeInventoryObject("orchid")
		gotoState(this, "happy")
		gotoState("cajun.alligator_4_inset.inset", "happy")
		stripmineGoodItemAction("orchid")
	else
		showHint("%那条鳄鱼似乎对它不感兴趣.%")
		stripmineWrongItemAction("orchid")
	end
end
---happy
onStateEnter = function (this)
	gotoLocation("cajun.swamp_house")
	showHint("%这个幽灵似乎满足了.%")
	gotoState("cajun.swamp_house.alligator_4", "inactive")
	setDefaultCursor (this)
end
