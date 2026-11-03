---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	showHint("%幽灵堵住了大门. 杰西说我必须找到她的魔法书. 上面有驱逐幽灵的法术.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "mixture_banish_spirits" then
		gotoState(this, "inactive")
		removeInventoryObject("mixture_banish_spirits")
		playSound("/sound/sfx/sfx_miasma_remove")
		gotoState("jessie_home.parlor.window", "active")
		stripmineGoodItemAction("mixture_banish_spirits")
	else
		showHint("%幽灵堵住了大门. 杰西说我必须找到她的魔法书. 上面有驱逐幽灵的法术.%")
		stripmineWrongItemAction("mixture_banish_spirits")
	end
end
---inactive
onStateEnter = function (this)
	setVisible (this, false)
	setInteractive(this, false)
	setDefaultCursor(this)
	gotoCutscene("ch2_opening", "../cs_finish", true)
	gotoState("jessie_home.front_gate.to_parlor", "active")
	gotoState("jessie_home.front_gate.house", "active")
	rechargeHintBottle()
	setCurrentChapter(2)
end
