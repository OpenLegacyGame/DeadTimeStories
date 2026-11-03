---active
onStateEnter = function (this)
	setTexture(this, "snake_01")
	setInteractive(this, true)
end

---click1
onStateEnter = function (this)
	setTexture(this, "snake_02")
end

---click2
onStateEnter = function (this)
	setTexture(this, "snake_03")
end

---click4
onStateEnter = function (this)
	setDefaultCursor (this)
	setTexture(this, "snake_03")
end

---sleep
onStateEnter = function (this)
	setTexture(this, "snake_01")
	setInteractive(this, true)
	setDefaultCursor (this)
	gotoState("jessie_home.shed.snake", "inactive")
end
