---wait1
onStateEnter = function(this)
	setTimer(this, 41)
end

onTimer = function(this)
	gotoState(this, "play1")
end

---play1
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_water_splash")
	gotoState(this, "wait2")
end

---wait2
onStateEnter = function(this)
	setTimer(this, 42)
end

onTimer = function(this)
	gotoState(this, "play2")
end

---play2
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_water_splash")
	gotoState(this, "wait1")
end

---inactive
