---wait1
onStateEnter = function(this)
	setTimer(this, 15)
end

onTimer = function(this)
	gotoState(this, "play1")
end

---play1
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_fire_place")
	gotoState(this, "wait1")
end


---inactive
