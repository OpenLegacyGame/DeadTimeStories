---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	showHint("%那条鳄鱼的牙很少!%")
	playSound("/sound/sfx/sfx_croco_click")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "mirror" then
		removeInventoryObject("mirror")
		gotoState(this, "happy")
		gotoState("cajun.alligator_3_inset.inset", "happy")
		stripmineGoodItemAction("mirror")
	else
		showHint("%那条鳄鱼似乎对它不感兴趣.%")
		stripmineWrongItemAction("mirror")
	end
end
---happy
onStateEnter = function (this)
	gotoLocation("cajun.swamp_house")
	showHint("%这个幽灵似乎满足了.%")
	gotoState("cajun.swamp_house.alligator_3", "inactive")
	setDefaultCursor (this)
end
