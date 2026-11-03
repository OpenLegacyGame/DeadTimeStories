---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this, true)
	setVisible(this, false)
end

---eating
onStateEnter = function (this)
	setVisible(this, true)
	playSound("/sound/sfx/sfx_cat_small")
end