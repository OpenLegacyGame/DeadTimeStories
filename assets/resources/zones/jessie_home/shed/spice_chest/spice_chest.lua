---closed
onStateEnter = function (this)
	setCursor(this, "view") 
	setInteractive(this, true)
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.spice_box_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---plated
onStateEnter = function (this)
	setTexture(this, "box_plated")
	setCursor(this, "view") 
	setInteractive(this, true)
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.spice_box_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---open
onStateEnter = function (this)
	setTexture(this, "box_opened")
	setCursor(this, "view") 
	setInteractive(this, true)
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.spice_box_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---used
onStateEnter = function (this)
	setDefaultCursor(this, true)
	setInteractive(this, false)
end