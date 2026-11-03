---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
end
---active
onStateEnter = function (this)
	setTexture(this, "front_gate_carriage")
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "go_up")
end
onLMouseUp = function (this)
	gotoCarriage()
	if math.random() >0.5 then
		playSound("/sound/sfx/carriage/ed_carriage_1")
	else
		playSound("/sound/sfx/carriage/ed_carriage_5")
	end
end