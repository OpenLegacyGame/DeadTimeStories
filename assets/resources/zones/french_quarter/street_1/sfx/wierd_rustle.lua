---wait
onStateEnter = function(this)
	setTimer(this, 17)
end

onTimer = function(this)
	
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_wierd_rustle2")
	gotoState(this, "wait")
end

---inactive
