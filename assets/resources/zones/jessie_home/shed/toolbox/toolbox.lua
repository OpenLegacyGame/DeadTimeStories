---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setTexture(this, "toolbox_closed")
end
onLMouseUp = function (this)
	showHint("%It's locked! But where is the knob?%")
	playSound("/sound/sfx/sfx_locked")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "knob" then
		gotoState(this, "open")
		gotoState("jessie_home.shed.spade_handle", "active")
		removeInventoryObject("knob")
		playSound("/sound/sfx/sfx_toolbox_open")
		gotoState("jessie_home.shed.fx_toolbox_changing", "active")
		stripmineGoodItemAction("knob")
	else
		stripmineWrongItemAction("knob")
	end
end
---open
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setTexture(this, "toolbox_opened")
end