---wait1
onStateEnter = function(this)
	setTimer(this, 11)
end

onTimer = function(this)
	gotoState(this, "play1")
end

---play1
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_croco_presence")
	gotoState(this, "wait1")
end

---inactive
