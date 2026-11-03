---active
onStateEnter = function (this)
	setVisible(this, false)
end

---ready
onStateEnter = function (this)
	setVisible(this, true)
end

---opened
onStateEnter = function (this)
	setTexture(this, "skulls_open")
end