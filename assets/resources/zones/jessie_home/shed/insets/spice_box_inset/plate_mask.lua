---closed
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	showHint("%这个盒子打不开. 它的盖上似乎缺了一块.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "brass" then
		gotoState(this, "plated")
		gotoState("jessie_home.spice_box_inset.inset", "plated")
		gotoState("jessie_home.shed.spice_chest", "plated")
		removeInventoryObject("brass")
		gotoState("jessie_home.shed.fx_spice_chest_changing", "active")
		stripmineGoodItemAction("brass")
	else
		showHint("%这个盒子打不开. 它的盖上似乎缺少了一片六角形的板.%")
		stripmineWrongItemAction("brass")
	end
end

---plated
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState(this, "open")
	gotoState("jessie_home.spice_box_inset.clove", "active")
	gotoState("jessie_home.spice_box_inset.mint", "active")	
	gotoState("jessie_home.spice_box_inset.inset", "open")
	gotoState("jessie_home.shed.spice_chest", "open")
	playSound("/sound/sfx/sfx_box_open")
end

---open
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this, true)
end















---inactive

















---active
onStateEnter = function (this)
	if not hasInventoryObject("mint") then
			gotoState("jessie_home.spice_box_inset.mint", "active")
		end
		if not hasInventoryObject("clove") then
			gotoState("jessie_home.spice_box_inset.clove", "active")
	end
end
