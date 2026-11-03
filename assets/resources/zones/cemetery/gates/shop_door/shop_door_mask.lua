---missing_bell
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	showHint("%门铃不见了.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "bell" then
		removeInventoryObject("bell")
		gotoState(this, "bell_on")
		gotoState("cemetery.gates.shop_door", "assembled")
		showHint("%铃铛非常合适. 也许现在我可以进去了??%")
		playSound("/sound/sfx/sfx_inv_item_put")
		stripmineGoodItemAction("bell")
	else
		showHint("%这个东西不合适.%")
		stripmineWrongItemAction("bell")
	end
end

---bell_on
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	showHint("%铃铛非常合适. 也许现在我可以进去了??%")
	playSound("/sound/sfx/sfx_door_bell")
	gotoState(this, "open")
	gotoState("cemetery.gates.shop_door", "open")
end

---open
onStateEnter = function (this)
	setCursor(this, "go_up")
end
onLMouseUp = function (this)
	gotoLocation("cemetery.voodoo_shop") 
	playSound("/sound/sfx/sfx_foot_concrete")
end