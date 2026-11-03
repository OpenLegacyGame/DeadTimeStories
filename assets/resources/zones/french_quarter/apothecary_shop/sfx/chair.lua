---wait
onStateEnter = function(this)
	setTimer(this, 12)
end

onTimer = function(this)
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_rocking_chair")
	gotoState(this, "wait")
end

---inactive
