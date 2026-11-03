---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
	setVisible("french_quarter.street_1.fx_blink_street_clock", true) 
end
onLMouseUp = function(this)
	showHint("%那个钟表看起来很滑稽. 表针不见了.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "clock_hands" then
		removeInventoryObject("clock_hands")
		gotoState(this, "assembled")
		stripmineGoodItemAction("clock_hands")
		playSound("/sound/sfx/sfx_inv_item_put")
	else
		showHint("%我为什么要把它放在钟表上?%")
		stripmineWrongItemAction("clock_hands")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end

---assembled
onStateEnter = function (this)
	setVisible("french_quarter.street_1.street_clock_hands", true)
	setCursor(this, "interact")
	setInteractive(this, true)
end
onLMouseUp = function(this)
	gotoState(this, "opened")
	playSound("/sound/sfx/sfx_mailbox_open")
end

---opened
onStateEnter = function (this)
	setVisible("french_quarter.street_1.street_clock_lid", true)
	setVisible("french_quarter.street_1.fx_blink_street_clock", false)
	setVisible("french_quarter.street_1.street_clock_hands", false)
	setCursor(this, "view")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "binoculars" then
		gotoInSet("french_quarter.street_clock_inset")
		stripmineGoodItemAction("binoculars")
		playSound("/sound/sfx/sfx_zoom_in")
	else
		showHint("%这个不能帮我看清写在钟表上的字!%")
		stripmineWrongItemAction("binoculars")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end
onLMouseUp = function(this)
	showHint("%有一些字写在钟表上, 但是我从这里看不清楚.%")
end