---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	if isInState("cemetery.voodoo_shop.bird_fish_trigger", "triggered") then
		showHint("%如果我好好跟它说, 也许这只鸟会把鱼给我? 不可能.%")
	else
		showHint("%这只鸟捉住了一条鱼, 但它并没有吃下去. 也许它想留着晚点再吃?%")
	end
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "slingshot" then
		removeInventoryObject("slingshot")
		gotoState("cajun.swamp_island.fish", "active")
		gotoState("cajun.swamp_island.sticks", "active")
		gotoState(this, "used")
		gotoState("cajun.swamp_island.bird", "used")
		showHint("%鸟鸣叫着飞走了. 它扔下了鱼和两根棍子.%")
		stripmineGoodItemAction("slingshot")
		playSound("/sound/sfx/sfx_raven_far")
	else
		showHint("%我不能在鸟身上使用这个!%")
		stripmineWrongItemAction("slingshot")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end

---used
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, false)
	setVisible("cajun.swamp_island.fx_blink_bird", false)
end