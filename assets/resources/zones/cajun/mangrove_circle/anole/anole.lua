---inactive
onStateEnter = function (this)
	setVisible(this, false)
end

---active
onStateEnter = function (this)
	setVisible(this, true)
	setTexture(this, "anole")
end

---hidden
onStateEnter = function (this)
	setTexture(this, "anole_hidden")
end

---eating
onStateEnter = function (this)
	setTexture(this, "anole_eating")
end

---clicked
onStateEnter = function (this)
	setVisible(this, false)
end