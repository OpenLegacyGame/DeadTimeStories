---closed
onStateEnter = function (this)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	showHint("%The door bell is missing%")
	playSound("/sound/sfx/sfx_item_use_fail")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "bell" then
		removeInventoryObject("bell")
		gotoState(this, "active")
		showHint("%The door bell is on it's place. I can proceed%")
		playSound("/sound/sfx/sfx_inv_item_put")
		stripmineGoodItemAction("bell")
	else
		showHint("%This item doesn't fit%")
		stripmineWrongItemAction("bell")
	end
end

---active
onStateEnter = function (this)
	setCursor(this, "go_up")
end
onLMouseUp = function (this)
	gotoLocation("cemetery.voodoo_shop") 
	playSound("/sound/sfx/sfx_foot_concrete")
end