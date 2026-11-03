---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "go_left")
end
onLMouseUp = function (this)
	gotoCarriage()
	if math.random() >0.5 then
		playSound("/sound/sfx/carriage/ed_carriage_4")
	else
		playSound("/sound/sfx/carriage/ed_carriage_8")
	end
end