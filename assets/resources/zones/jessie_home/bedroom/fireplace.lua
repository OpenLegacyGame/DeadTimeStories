---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	showHint("%没有什么比一个空壁炉看上去更冷的了.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "matches" then
		showHint("%我无法把火点燃 - 这里没有木头.%")
		stripmineGoodItemAction("matches")
	else
		stripmineWrongItemAction("matches")
	end
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
end
