---locked
onStateEnter = function (this)
	setTexture(this, "gazebo")
	setInteractive(this, false)
end

---unlocked
onStateEnter = function (this)
	setVisible(this, false)
end