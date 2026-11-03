---inactive
onStateEnter = function(this)
	setVisible(this, false)
end

onStateLeave = function(this)
	setVisible(this, true)
end

---active
onStateEnter = function(this)
	setTimer(this, 2)
end

onTimer = function(this)
	gotoState(this, "inactive")
end