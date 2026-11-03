---active
onStateEnter = function (this)
	setVisible("jessie_home.kitchen.fx_candles", false)
	setVisible("jessie_home.kitchen.fx_blink_lantern", true)
	setCursor(this, "interact") 
	setInteractive(this, true)
end

onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "wick" then
		gotoState("jessie_home.kitchen.lantern_wick", "visible")
		removeInventoryObject("wick")
		stripmineGoodItemAction("wick")
	end
	if inventoryObject == "oil" then
		gotoState("jessie_home.kitchen.lantern_oil", "visible")
		removeInventoryObject("oil")
		stripmineGoodItemAction("oil")
	end
	if isInState("jessie_home.kitchen.lantern_wick", "visible") and
		isInState("jessie_home.kitchen.lantern_oil", "visible") then
		gotoState(this, "assembled")
	end
	if inventoryObject == "matches" then
		if (not isInState("jessie_home.kitchen.lantern_wick", "visible") and
			not isInState("jessie_home.kitchen.lantern_oil", "visible")) then
			showHint("%这是一盏油灯,但是它还缺少一些东西.%")
		end
		if (not isInState("jessie_home.kitchen.lantern_wick", "visible") and
			isInState("jessie_home.kitchen.lantern_oil", "visible")) then
			showHint("%我需要一根灯芯.%")
		end
		if (isInState("jessie_home.kitchen.lantern_wick", "visible") and
			not isInState("jessie_home.kitchen.lantern_oil", "visible")) then
			showHint("%里面没有油了.%")
		end
	end
end

onLMouseUp = function (this)
	if isMinigameFinishedHexBones() then
		showHint("%这是一盏油灯, 但是还缺少一些东西. 我不知道这里是不是我应该 '点亮火花' 的地方?%")
	else
		showHint("%这是一盏油灯, 但是还缺少一些东西. %")
	end
end

---assembled
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive(this, true)
	setVisible("jessie_home.kitchen.fx_blink_lantern", true)
end
onLMouseUp = function (this)
	showHint("%现在我可以点亮它了.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "matches" then
		gotoState(this, "lighten")
		stripmineGoodItemAction("lighten")
	elseif not inventoryObject ~= "matches" then
		showHint("%我不可能用这个点燃油灯.%")
		stripmineWrongItemAction("lighten")
	end
end

---lighten
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive(this, true)
	setTexture(this, "lantern_lightened")
	setVisible("jessie_home.kitchen.fx_candles", true)
	setVisible("jessie_home.kitchen.fx_blink_lantern", false)
	setTimer(this, 1)
end

onTimer = function(this)
	gotoCutscene("ch2_garlic", "/cutscenes/ch2_garlic/on_finish", true)
	gotoState("jessie_home.kitchen.garlic_head", "active")
	gotoState(this, "inactive")
	setVisible("jessie_home.parlor.fx_candles", true)
end

---inactive
onStateEnter = function (this)
	setVisible("jessie_home.kitchen.fx_candles", false)
	setVisible("jessie_home.kitchen.fx_blink_lantern", false)
	setDefaultCursor(this)
	setInteractive(this, false)
	setTexture(this, "lantern_broken")
	gotoState("jessie_home.kitchen.lantern_oil", "inactive")
	gotoState("jessie_home.kitchen.lantern_wick", "inactive")
	gotoState("jessie_home.kitchen.lantern_glass", "invisible")
	showHint("%哦, 太可怕了! 这栋房子的命运是被烧毁吗? 我猜我应该继续听杰西的故事去了解这一切都是为什么.%")
end