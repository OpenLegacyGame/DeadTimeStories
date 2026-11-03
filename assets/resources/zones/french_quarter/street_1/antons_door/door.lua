---closed
onStateEnter = function (this)
	setDefaultCursor(this, true)
	setInteractive (this, false)
	setVisible("french_quarter.street_1.fx_blink_antons_door", true) 
	setVisible(this, false)
end	

---opened
onStateEnter = function (this)
	setDefaultCursor(this, true)
	setInteractive (this, false)
	setVisible(this, true)
end
