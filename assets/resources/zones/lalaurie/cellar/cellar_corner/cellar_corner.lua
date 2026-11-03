---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("lalaurie.cellar.fx_flora_corner", true)
end
onLMouseUp = function (this)
	gotoHob("../../../lalaurie/HOB_cellar_corner.hob", "round0")
	playSound("/sound/sfx/sfx_zoom_in")
end

---active_1
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("lalaurie.cellar.fx_flora_corner", true)
end
onLMouseUp = function (this)
	gotoHob("../../../lalaurie/HOB_cellar_corner.hob", "round1")
	playSound("/sound/sfx/sfx_zoom_in")
end

---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("lalaurie.cellar.fx_flora_corner", false)
end