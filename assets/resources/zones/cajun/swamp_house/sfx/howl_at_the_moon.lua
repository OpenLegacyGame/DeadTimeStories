---wait
onStateEnter = function(this)
	setTimer(this, 43)
end

onTimer = function(this)
	
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_howl_at_the_moon")
	gotoState(this, "wait")
end

---inactive
