---inactive

---wait
onStateEnter = function(this)
	setTimer(this, 9)
end

onTimer = function(this)
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_clock")
	gotoState(this, "wait")
end
