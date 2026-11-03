---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setTexture(this, "gas_lamp_safe")
	setInteractive(this, true)
	setVisible("french_quarter.street_1.fx_blink_gas_lamp", true) 
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "cobble" then
		gotoState(this, "broken")
		playSound("/sound/sfx/sfx_hit_the_lamp")
		removeInventoryObject("cobble")
		stripmineGoodItemAction("cobble")
	elseif inventoryObject == "binoculars" then
		showHint("%灯后面的墙上写着些字. 也许我可以想办法把它移开?%")
	else
		showHint("%用这个东西有点笨拙.%")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end
onLMouseUp = function (this)
	showHint("%我想有些字写在灯后面, 但是我从这里看不清楚.%")
	playSound("/sound/sfx/sfx_item_use_fail")
end
---broken
onStateEnter = function (this)
	setCursor(this, "view") 
	setTexture(this, "gas_lamp_broken")
	setInteractive(this, true)
	showHint("%我把灯打碎了, 希望没人会介意.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "binoculars" then
		gotoInSet("french_quarter.gas_lamp_inset")
		stripmineGoodItemAction("binoculars")
		playSound("/sound/sfx/sfx_zoom_in")
	else
		showHint("%不, 这没用.%")
		stripmineWrongItemAction("binoculars")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end
onLMouseUp = function(this)
	showHint("%那里写着些字, 但是我看不清楚.%")
end
---used
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setTexture(this, "gas_lamp_broken")
	setVisible("french_quarter.street_1.fx_blink_gas_lamp", false) 
end