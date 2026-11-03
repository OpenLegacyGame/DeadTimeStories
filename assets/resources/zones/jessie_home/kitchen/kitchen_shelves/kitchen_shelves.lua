---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.kitchen.fx_flora_shelves", true)
end
onLMouseUp = function (this)
	gotoHob("../../../jessie_home/HOB_kitchen_shelves.hob", "round0")
end

---active_1
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.kitchen.fx_flora_shelves", true)
end
onLMouseUp = function (this)
	gotoHob("../../../jessie_home/HOB_kitchen_shelves.hob", "round1")
end

---active_2
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.kitchen.fx_flora_shelves", true)
end
onLMouseUp = function (this)
	gotoHob("../../../jessie_home/HOB_kitchen_shelves.hob", "round2")
end

---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("jessie_home.kitchen.fx_flora_shelves", false)
	if wasInventoryObjectFound("sponge") and not
		wasInventoryObjectFound("empty_jug") then
		gotoState(this, "active")		
	end 
end