---active
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
end

---opened
onStateEnter = function (this)
	setVisible(this, true)
end
