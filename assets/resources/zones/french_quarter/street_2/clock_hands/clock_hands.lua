---active
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible("french_quarter.street_2.fx_blink_clock_hands", true) 
end

---clicked
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
end