---wait
onStateEnter = function(this)
	setTimer(this, 41)
end

onTimer = function(this)
	
	gotoState(this, "play")
end

---play
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_cat_small")
	gotoState(this, "wait")
end

---inactive
