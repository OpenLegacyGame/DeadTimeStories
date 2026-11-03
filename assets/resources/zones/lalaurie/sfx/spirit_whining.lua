---wait1
onStateEnter = function(this)
	setTimer(this, 10)
end

onTimer = function(this)
	gotoState(this, "play1")
end

---play1
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_f_spirit_whining3")
	gotoState(this, "wait2")
end

---wait2
onStateEnter = function(this)
	setTimer(this, 10)
end

onTimer = function(this)
	gotoState(this, "play2")
end

---play2
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_f_spirit_whining2")
	gotoState(this, "wait3")
end

---wait3
onStateEnter = function(this)
	setTimer(this, 10)
end

onTimer = function(this)
	gotoState(this, "play3")
end

---play3
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_f_spirit_whining1")
	gotoState(this, "wait4")
end

---wait4
onStateEnter = function(this)
	setTimer(this, 10)
end

onTimer = function(this)
	gotoState(this, "play4")
end

---play4
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_m_spirit_whining2")
	gotoState(this, "wait5")
end

---wait5
onStateEnter = function(this)
	setTimer(this, 10)
end

onTimer = function(this)
	gotoState(this, "play5")
end

---play3
onStateEnter = function(this)
	playEnvSound("/sound/sfx/sfx_m_spirit_whining1")
	gotoState(this, "wait1")
end

---inactive
