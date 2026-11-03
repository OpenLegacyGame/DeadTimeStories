---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	showHint("%茶杯里有什么东西. 但是我可不能把手指伸进去 - 茶太烫了!%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "magnet" then
		if hasInventoryObject("thread") then
			removeInventoryObject("thread")
			addInventoryObject("sewing_kit")
			showMessageDialog("%你已经把针和线合成了缝纫工具.%")
		else
			addInventoryObject("needle")
		end
		gotoState(this, "inactive")
		removeInventoryObject("magnet")
		stripmineGoodItemAction("magnet")
		playSound("/sound/sfx/sfx_item_found")
	else
		showHint("%不, 这样没用.%")
		stripmineWrongItemAction("magnet")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end
---inactive
onStateEnter = function (this)
	gotoState("jessie_home.parlor.teacup", "used")
	gotoState("jessie_home.teacup_inset.needle", "invisible")
	gotoState("cajun.swamp_island.wringer_corner", "active")
	setCursor(this, "interact")
end
onLMouseUp = function(this)
	showHint("%肯定是幽灵令茶保持着这么热的温度, 但是我再也不需要从它里面拿东西了.%")
end