---wait1
onStateEnter = function(this)
	setTimer(this, 40)
end

onTimer = function(this)
	gotoState(this, "play1")
end

---play1
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_wind")
	gotoState(this, "wait3")
end

---wait3
onStateEnter = function(this)
	setTimer(this, 40)
end

onTimer = function(this)
	gotoState(this, "play3")
end

---play3
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_wind2")
	gotoState(this, "wait1")
end

---inactive
