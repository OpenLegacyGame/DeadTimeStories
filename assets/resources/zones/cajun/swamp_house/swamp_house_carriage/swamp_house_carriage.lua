---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "go_right")
end
onLMouseUp = function (this)
	if (wasInventoryObjectFound("baby_doll") and
		wasInventoryObjectFound("sticks") and
		wasInventoryObjectFound("fish")) then
		gotoState("jessie_home.front_gate.zone_trigger", "c6mid4")
	end
	gotoCarriage()
	if math.random() >0.5 then
		playSound("/sound/sfx/carriage/ed_carriage_8")
	else
		playSound("/sound/sfx/carriage/ed_carriage_4")
	end
end