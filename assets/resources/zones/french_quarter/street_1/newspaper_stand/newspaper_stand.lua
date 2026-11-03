---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive (this, true)
	setVisible("french_quarter.street_1.fx_blink_newspaper_stand", true) 
end
onLMouseUp = function(this)
	gotoInSet("french_quarter.newspaper_stand_inset")
end