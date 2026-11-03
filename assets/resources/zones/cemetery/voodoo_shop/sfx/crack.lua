---wait1
onStateEnter = function(this)
	setTimer(this, 31)
end

onTimer = function(this)
	gotoState(this, "play1")
end

---play1
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_v_floor_crack1")
	gotoState(this, "wait2")
end

---wait2
onStateEnter = function(this)
	setTimer(this, 33)
end

onTimer = function(this)
	gotoState(this, "play2")
end

---play2
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_v_floor_crack2")
	gotoState(this, "wait1")
end

---inactive
