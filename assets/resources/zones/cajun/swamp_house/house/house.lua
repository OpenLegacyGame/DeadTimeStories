---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
end
onLMouseUp = function (this)
	showHint("%那些鳄鱼堵着路. 曼波•玛丽说了一些关于 '安抚这些古老的沼泽恶魔'的事. 但是我该怎么做呢?%")
	playSound("/sound/sfx/sfx_item_use_fail")
	addInventoryObject("clippers")
end

---open
onStateEnter = function (this)
	setCursor(this, "view") 
	setInteractive(this, true)
	gotoState("cajun.swamp_house.sfx_croco_presence", "inactive")
end
onLMouseUp = function (this)
	if not hasInventoryObject("boat_hook") then
		gotoState("cajun.house_inset.boat_hook", "active")
	end
	if hasInventoryObject("blue_anole") then
		gotoState("cajun.house_inset.anole_place", "active")
	end
	gotoInSet("cajun.house_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---used
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
end
