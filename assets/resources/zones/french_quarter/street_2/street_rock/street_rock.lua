---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
	setVisible("french_quarter.street_2.fx_blink_street_rock", true) 
end
onLMouseUp = function(this)
	showHint("%那个轮子上有一个形状很奇怪的凹槽.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "horseshoe" then
		removeInventoryObject("horseshoe")
		gotoState(this, "used")
		stripmineGoodItemAction("horseshoe")
		playSound("/sound/sfx/sfx_inv_item_put")
	else
		showHint("%这件东西不合适.%")
		stripmineWrongItemAction("horseshoe")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end

---used
onStateEnter = function (this)
	setVisible("french_quarter.street_2.horseshoe", true)
	setVisible("french_quarter.street_2.bessie", true)
	setCursor(this, "view")
	setInteractive(this, true)
	setVisible("french_quarter.street_2.fx_blink_street_rock", false) 
end
onLMouseUp = function(this)
	showHint("%那里写着些字, 但是我看不清楚.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "binoculars" then
		gotoInSet("french_quarter.bessie_inset")
		stripmineGoodItemAction("binoculars")
		playSound("/sound/sfx/sfx_zoom_in")
	else
		showHint("%不, 这没用.%")
		stripmineWrongItemAction("binoculars")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end
