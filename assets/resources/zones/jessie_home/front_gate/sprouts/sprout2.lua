---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	setDefaultCursor(this)
end 

---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	if not isPlantBookEnabled() then
		showHint("%我不是专家, 所以无法认出那些植物. 也许这附近有什么东西能帮助我识别植物.%")
	else
		showHint("%我需要某件东西来剪断坚硬的茎.%")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end
onInventoryApplied = function (this, inventoryObject)
	playSound("/sound/sfx/sfx_clippers_use")
	if inventoryObject == "clippers" then
		if isPlantBookEnabled() then
			gotoState("jessie_home.front_gate.fx_sprout2_changing", "active")
			gotoState(this, "inactive")
			gotoState("jessie_home.front_gate.sprouts", "inactive")
			addInventoryObject("solomon_seal")
			playSound("/sound/sfx/sfx_item_found")
			showHint("%这就是黄精!%")
			stripmineGoodItemAction("clippers")
		else
			showHint("%我不是专家, 所以无法认出那些植物. 也许这附近有什么东西能帮助我识别植物.%")
			playSound("/sound/sfx/sfx_item_use_fail")
			stripmineWrongItemAction("clippers")
		end
	else
		showHint("%我需要某件东西来剪断坚硬的茎.%")
		stripmineWrongItemAction("clippers")
	end
end
