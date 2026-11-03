---wait
onStateEnter = function(this)
	setTimer(this, 43)
end

onTimer = function(this)
	
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_bats")
	gotoState(this, "wait")
end

---inactive
