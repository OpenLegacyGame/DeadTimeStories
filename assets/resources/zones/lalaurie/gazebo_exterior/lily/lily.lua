---active
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, false)
end

---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setDefaultCursor(this)
end 
