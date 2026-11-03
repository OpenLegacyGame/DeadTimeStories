---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	setDefaultCursor(this)
end 

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
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
	if inventoryObject == "clippers" then
		if isPlantBookEnabled() then
			gotoState("jessie_home.front_gate.fx_sprout3_changing", "active")
			gotoState(this, "inactive")
			showHint("%我不需要这种植物来制作法术. 但是在这一簇里也许有我需要的.%")
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
