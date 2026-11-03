---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
end

---active
onStateEnter = function (this)
	setCursor(this, "go_up")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	gotoLocation("jessie_home.parlor") 
	playSound("/sound/sfx/sfx_foot_wood")
end
