---wait
onStateEnter = function(this)
	setTimer(this, 25)
end

onTimer = function(this)
	
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_draft3")
	gotoState(this, "wait")
end

---inactive
