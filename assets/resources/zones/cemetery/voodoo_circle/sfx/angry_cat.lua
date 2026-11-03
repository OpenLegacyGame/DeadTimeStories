---wait
onStateEnter = function(this)
	setTimer(this, 26)
end

onTimer = function(this)
	
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_angry_cat")
	gotoState(this, "wait")
end

---inactive
