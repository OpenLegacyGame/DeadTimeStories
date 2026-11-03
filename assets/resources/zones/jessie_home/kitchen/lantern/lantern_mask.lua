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
		playSound("/sound/sfx/sfx_wick_use")
	end
	if inventoryObject == "oil" then
		gotoState("jessie_home.kitchen.lantern_oil", "visible")
		removeInventoryObject("oil")
		stripmineGoodItemAction("oil")
		playSound("/sound/sfx/sfx_oil_use")
	end
	if isInState("jessie_home.kitchen.lantern_wick", "visible") and
		isInState("jessie_home.kitchen.lantern_oil", "visible") then
		gotoState("jessie_home.kitchen.lantern", "assembled")
		gotoState(this, "assembled")
	end
	if inventoryObject == "matches" then
		if (not isInState("jessie_home.kitchen.lantern_wick", "visible") and
			not isInState("jessie_home.kitchen.lantern_oil", "visible")) then
			showHint("%我需要一根灯芯.%")
		end
		if (not isInState("jessie_home.kitchen.lantern_wick", "visible") and
			isInState("jessie_home.kitchen.lantern_oil", "visible")) then
			showHint("%我需要一根灯芯.%")
		end
		if (isInState("jessie_home.kitchen.lantern_wick", "visible") and
			not isInState("jessie_home.kitchen.lantern_oil", "visible")) then
			showHint("%里面没油了.%")
		end
	end
end

onLMouseUp = function (this)
	if (not isInState("jessie_home.kitchen.lantern_wick", "visible") and
		isInState("jessie_home.kitchen.lantern_oil", "visible")) then
		showHint("%我需要一根灯芯.%")
	end
	if (isInState("jessie_home.kitchen.lantern_wick", "visible") and
		not isInState("jessie_home.kitchen.lantern_oil", "visible")) then
		showHint("%里面没有油了.%")
	end
	if (not isInState("jessie_home.kitchen.lantern_wick", "visible") and
		not isInState("jessie_home.kitchen.lantern_oil", "visible")) then
		if math.random() > 0.5 then
			showHint("%在它能使用之前我不能点燃油灯. 它还缺少一些东西.%")
		else
			showHint("%我需要一根灯芯和一些油.%")
		end
	end	
end

---assembled
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive(this, true)
	setVisible("jessie_home.kitchen.fx_blink_lantern", true)
end
onLMouseUp = function (this)
	showHint("%现在我可以点燃它了.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "matches" then
		gotoState(this, "lighten")
		gotoState("jessie_home.kitchen.lantern", "lighten")
		stripmineGoodItemAction("lighten")
		playSound("/sound/sfx/sfx_matches_use")
	else
		showHint("%我不能用这个点燃油灯.%")
		stripmineWrongItemAction("lighten")
	end
end

---lighten
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive(this, true)
	setTexture("jessie_home.kitchen.lantern", "lantern_lightened")
	setVisible("jessie_home.kitchen.fx_candles", true)
	setVisible("jessie_home.kitchen.fx_blink_lantern", false)
	removeInventoryObject("matches")
	setTimer(this, 1)
end

onTimer = function(this)
	gotoCutscene("ch2_garlic", "/cutscenes/ch2_garlic/on_finish", true)
	gotoState("jessie_home.kitchen.garlic_head", "active")
	gotoState(this, "inactive")
	gotoState("jessie_home.kitchen.lantern", "inactive")
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
	showHint("%哦, 太可怕了! 这栋房子的命运是被烧毁吗? 那是怎么发生的?%")
end