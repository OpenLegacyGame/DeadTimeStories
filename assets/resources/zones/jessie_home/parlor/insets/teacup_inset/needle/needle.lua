---visible
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, false)
end

---invisible
onStateEnter = function (this)
	setVisible(this, false)
end