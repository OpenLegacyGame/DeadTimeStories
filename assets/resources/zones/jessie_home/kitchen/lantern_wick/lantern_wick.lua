---invisible
onStateEnter = function (this)
	setVisible(this, false)
	setDefaultCursor(this)
	setInteractive(this, false)
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "wick" then
		gotoState("lantern_wick", "visible")
		gotoState(this, "active")
		removeInventoryObject("wick")
		stripmineGoodItemAction("wick")
	else
		stripmineWrongItemAction("wick")
	end
end
---visible
onStateEnter = function (this)
	setVisible(this, true)
end
---inactive
onStateEnter = function (this)
	setVisible(this, false)
end