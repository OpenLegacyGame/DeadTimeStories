---wait
onStateEnter = function(this)
	setTimer(this, 17)
end

onTimer = function(this)
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_shadow")
	gotoState(this, "wait")
end

---inactive
