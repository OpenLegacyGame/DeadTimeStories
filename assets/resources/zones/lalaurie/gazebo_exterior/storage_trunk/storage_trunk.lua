---active
onStateEnter = function (this)
	setInteractive(this, true)
	setDefaultCursor(this, true)
	setVisible(this, false)
end

---open
onStateEnter = function (this)
	setVisible(this, true)
	setDefaultCursor(this) 
end