---inactive
onStateEnter = function (this)
	setVisible(this, false)
end
---active
onStateEnter = function (this)
	setVisible(this, true)
	setTimer(this, 2)
end
onTimer = function(this)
	gotoState(this, "inactive")
	gotoState("jessie_home.kitchen.water_pump_handle", "active")
	if isInState("jessie_home.kitchen.water_pump_jug", "empty" )  then
		gotoState("jessie_home.kitchen.water_pump_jug", "full")
		gotoState("jessie_home.kitchen.fx_water_pump_jug_changing", "active")
	end
end
