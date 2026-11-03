---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	showHint("%哇, 那条鳄鱼是个怪物! 我怎么才能赶走它呢?%")
	playSound("/sound/sfx/sfx_croco_click")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "toy_trumpet" then
		removeInventoryObject("toy_trumpet")
		gotoState(this, "happy")
		gotoState("cajun.alligator_1_inset.inset", "happy")
		stripmineGoodItemAction("toy_trumpet")
	else
		showHint("%那条鳄鱼似乎对这个不感兴趣.%")
		stripmineWrongItemAction("toy_trumpet")
	end
end
---happy
onStateEnter = function (this)
	showHint("%似乎这个幽灵满足了.%")
	gotoState("cajun.swamp_house.alligator_1", "inactive")
	setDefaultCursor (this)
	gotoLocation("cajun.swamp_house")
end
