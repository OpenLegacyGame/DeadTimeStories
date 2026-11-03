---active
onStateEnter = function (this)
	setInteractive (this, true)
	setDefaultCursor(this)
end

---used
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, false)
end