---invisible
onStateEnter = function (this)
	setVisible(this, false)
	setDefaultCursor(this)
	setInteractive(this, false)
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "oil" then
		gotoState("lantern_oil", "visible")
		gotoState(this, "active")
		removeInventoryObject("oil")
		stripmineGoodItemAction("oil")
	else
		stripmineWrongItemAction("oil")
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