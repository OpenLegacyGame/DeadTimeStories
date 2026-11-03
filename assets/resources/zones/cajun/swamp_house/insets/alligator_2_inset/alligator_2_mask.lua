---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	showHint("%那条鳄鱼看上去有点疯狂. 你该怎样安抚一条疯狂的鳄鱼?%")
	playSound("/sound/sfx/sfx_croco_click")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "moonshine" then
		removeInventoryObject("moonshine")
		gotoState(this, "happy")
		gotoState("cajun.alligator_2_inset.inset", "happy")
		stripmineGoodItemAction("moonshine")
	else
		showHint("%那条鳄鱼似乎对它不感兴趣.%")
		stripmineWrongItemAction("moonshine")
	end
end
---happy
onStateEnter = function (this)
	gotoLocation("cajun.swamp_house")
	showHint("%似乎这个幽灵满足了.%")
	gotoState("cajun.swamp_house.alligator_2", "inactive")
	setDefaultCursor (this)
end
