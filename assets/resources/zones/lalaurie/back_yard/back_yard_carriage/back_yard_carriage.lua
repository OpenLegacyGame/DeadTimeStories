---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "go_up")
end
onLMouseUp = function (this)
	if (wasInventoryObjectFound("lipstick") and
		wasInventoryObjectFound("lalauries_dress_piece") and
		wasInventoryObjectFound("lalauries_hair")) then
		gotoState("jessie_home.front_gate.zone_trigger", "c6mid3")
	end
	gotoCarriage()
	if math.random() >0.5 then
		playSound("/sound/sfx/carriage/ed_carriage_5")
	else
		playSound("/sound/sfx/carriage/ed_carriage_6")
	end
end