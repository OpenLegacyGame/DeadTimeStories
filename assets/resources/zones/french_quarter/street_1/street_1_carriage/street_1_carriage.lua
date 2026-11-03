---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "go_up")
end
onLMouseUp = function (this)
	if (wasInventoryObjectFound("two_buttons") and
		wasInventoryObjectFound("muslin_bandage")) then
		gotoState("jessie_home.front_gate.zone_trigger", "c6mid5")
	end
	gotoCarriage()
	if math.random() >0.5 then
		playSound("/sound/sfx/carriage/ed_carriage_1")
	else
		playSound("/sound/sfx/carriage/ed_carriage_8")
	end
end