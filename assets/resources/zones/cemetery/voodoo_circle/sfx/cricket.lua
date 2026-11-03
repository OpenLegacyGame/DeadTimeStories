---wait
onStateEnter = function(this)
	setTimer(this, 13)
end

onTimer = function(this)
	
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_cricket")
	gotoState(this, "wait")
end

---inactive
