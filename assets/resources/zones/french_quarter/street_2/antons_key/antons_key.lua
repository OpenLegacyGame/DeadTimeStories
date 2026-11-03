---cut
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	setVisible("french_quarter.street_2.fx_blink_antons_key", false)
end


---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setDefaultCursor(this, true)
	setInteractive(this, false)
	setVisible("french_quarter.street_2.fx_blink_antons_key", false) 
end

---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("french_quarter.street_2.fx_blink_antons_key", true) 
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
	addInventoryObject("antons_key")
	setVisible("french_quarter.street_2.fx_blink_antons_key", false)
end