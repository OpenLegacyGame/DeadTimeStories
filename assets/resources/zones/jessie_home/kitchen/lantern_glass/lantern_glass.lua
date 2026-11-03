---visible
onStateEnter = function (this)
	setVisible(this, true)
end

---invisible
onStateEnter = function (this)
	setVisible(this, false)
	setDefaultCursor(this)
	setInteractive(this, false)
end