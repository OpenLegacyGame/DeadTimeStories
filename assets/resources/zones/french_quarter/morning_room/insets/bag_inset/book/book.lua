---preactive
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function(this)
	showHint("%这本书列出了毒药和相应的解毒剂. 我也许能制作出解毒剂, 但我得先知道杰西给安东夫人下了什么毒.%")
	gotoState(this, "active")
	playSound("/sound/sfx/sfx_book_open")
end

---active
onStateEnter = function (this)
	setTexture(this, "book_opened")
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function(this)
	if wasInventoryObjectFound("aconite") then
		gotoState(this, "inactive")
		gotoState("jessie_home.front_gate.to_HOB_plants", "active4")
		gotoState("jessie_home.shed.spellmixing", "active4")
		enableSpell("aconite_antidote")
		showMessageDialog("%杰西给安东夫人下了乌头毒药. 我已经找到了解毒剂的配方... 我得记在魔法书里.%")
	else
		showHint("%这本书列出了毒药和相应的解毒剂. 我也许能制作出解毒剂, 但我得先知道杰西给安东夫人下了什么毒.%")
	end
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "aconite" then
		gotoState(this, "inactive")
		gotoState("jessie_home.front_gate.to_HOB_plants", "active4")
		gotoState("jessie_home.shed.spellmixing", "active4")
		enableSpell("aconite_antidote")
		showMessageDialog("%杰西给安东夫人下了乌头毒药. 我已经找到了解毒剂的配方... 我得记在魔法书里.%")
		stripmineGoodItemAction("aconite")
	else
		showHint("%This is not a poison.%")
		stripmineWrongItemAction("aconite")
	end
end

---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, true)
end
