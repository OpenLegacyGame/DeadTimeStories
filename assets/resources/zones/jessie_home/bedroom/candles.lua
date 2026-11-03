---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("jessie_home.bedroom.fx_blink_candles", true)
end
onLMouseUp = function (this)
	showHint("%我需要什么东西来点燃蜡烛.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "matches" then
		gotoState("jessie_home.bedroom.fx_candles", "active")
		gotoState(this, "inactive")		
		stripmineGoodItemAction("matches")
	else
		showHint("%这没用.%")
		stripmineWrongItemAction("matches")
	end	
end
---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible("jessie_home.bedroom.fx_blink_candles", false)
end
