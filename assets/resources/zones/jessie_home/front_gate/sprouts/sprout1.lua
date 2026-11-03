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
		showHint("%我需要某件东来西剪断坚硬的茎.%")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "clippers" then
		if isPlantBookEnabled() then
			gotoState("jessie_home.front_gate.fx_sprout1_changing", "active")
			gotoState(this, "inactive")
			showHint("%这种植物看上去不像是法术所需要的任何一种.%")
			stripmineGoodItemAction("clippers")
		else
			showHint("%我不是专家, 所以无法认出那些植物. 也许这附近有什么东西能帮助我识别植物.%")
			playSound("/sound/sfx/sfx_item_use_fail")
			stripmineWrongItemAction("clippers")
		end
	end	
end
