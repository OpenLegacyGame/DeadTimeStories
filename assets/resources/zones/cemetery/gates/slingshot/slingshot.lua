---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setDefaultCursor(this, true)
	setInteractive(this, false)
	setVisible("cemetery.gates.fx_blink_slingshot", false) 
end

---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("cemetery.gates.fx_blink_slingshot", true)
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end

---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("slingshot")
	setVisible("cemetery.gates.fx_blink_slingshot", false)
end