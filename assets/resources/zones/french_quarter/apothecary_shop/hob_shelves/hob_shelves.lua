---inactive
onStateEnter = function (this)
	setInteractive(this, false)
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("french_quarter.apothecary_shop.fx_flora_hob_shelves", true)
end
onLMouseUp = function (this)
	gotoHob("../../../french_quarter/HOB_apothecary_shelves.hob", "round0")
end

---active_1
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("french_quarter.apothecary_shop.fx_flora_hob_shelves", true)
end
onLMouseUp = function (this)
	gotoHob("../../../french_quarter/HOB_apothecary_shelves.hob", "round1")
end

---active_2
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("french_quarter.apothecary_shop.fx_flora_hob_shelves", true)
end
onLMouseUp = function (this)
	gotoHob("../../../french_quarter/HOB_apothecary_shelves.hob", "round2")
end

---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("french_quarter.apothecary_shop.fx_flora_hob_shelves", false)
	if wasInventoryObjectFound("aspirin") and isMinigameFinishedSortApothecaryItems() and
		not wasInventoryObjectFound("baking_soda") then
		gotoState(this, "active")		
	end
	if wasInventoryObjectFound("baking_soda") and isMinigameFinishedSortApothecaryItems() and
		not wasInventoryObjectFound("aspirin") then
		gotoState(this, "active_1")
	end

end