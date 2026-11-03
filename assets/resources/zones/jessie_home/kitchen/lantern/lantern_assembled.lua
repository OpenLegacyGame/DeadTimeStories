---inactive
onStateEnter = function (this)
	setInteractive(this, false)
end
---assembled
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	showHint("%现在我可以点亮它了.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "matches" then
		gotoState("jessie_home.kitchen.lantern", "lighten")
		gotoState(this, "inactive")
		stripmineGoodItemAction("matches")
	elseif inventoryObject ~= "matches" then
		showHint("%我不可能用这个点燃油灯.%")
		stripmineWrongItemAction("matches")
	end
end
