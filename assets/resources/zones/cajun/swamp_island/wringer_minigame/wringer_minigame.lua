---active
onStateEnter = function (this)
	setTexture(this, "wringer_minigame")
end

---inactive
onStateEnter = function (this)
	setVisible(this, false)
end