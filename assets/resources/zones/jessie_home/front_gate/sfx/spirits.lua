---wait
onStateEnter = function(this)
	setTimer(this, 22)
end

onTimer = function(this)
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_spirits")
	gotoState(this, "wait")
end

---inactive
