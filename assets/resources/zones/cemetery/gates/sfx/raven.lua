---wait
onStateEnter = function(this)
	setTimer(this, 31)
end

onTimer = function(this)
	
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_raven")
	gotoState(this, "wait")
end

---inactive
