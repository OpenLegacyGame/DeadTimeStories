---wait1
onStateEnter = function(this)
	setTimer(this, 41)
end

onTimer = function(this)
	gotoState(this, "play1")
end

---play1
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_r_blink_eyes1")
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
	playEnvSound("/sound/sfx/sfx_r_blink_eyes2")
	gotoState(this, "wait3")
end

---wait3
onStateEnter = function(this)
	setTimer(this, 43)
end

onTimer = function(this)
	gotoState(this, "play3")
end

---play3
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_r_blink_eyes3")
	gotoState(this, "wait1")
end

---inactive
