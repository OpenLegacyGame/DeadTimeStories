---preactive
onStateEnter = function (this)
	setInteractive(this, true)
end

---active
onStateEnter = function (this)
	setTexture(this, "bag_closed")
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_anole_hiding")
end

---clicked
onStateEnter = function (this)
	setTexture(this, "bag_opened")
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function (this)
	gotoInSet("french_quarter.bag_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---inactive
onStateEnter = function (this)
	setInteractive(this, false)
end