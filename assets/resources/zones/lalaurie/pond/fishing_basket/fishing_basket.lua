---inactive
onStateEnter = function (this)
	setInteractive(this, false)
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("lalaurie.pond.fx_flora_basket", true)
end
onLMouseUp = function (this)
	gotoHob("../../../lalaurie/HOB_fishing_basket.hob", "round0")
	playSound("/sound/sfx/sfx_zoom_in")
end

---active_1
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("lalaurie.pond.fx_flora_basket", true)
end
onLMouseUp = function (this)
	gotoHob("../../../lalaurie/HOB_fishing_basket.hob", "round1")
	playSound("/sound/sfx/sfx_zoom_in")
end

---active_2
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("lalaurie.pond.fx_flora_basket", true)
end
onLMouseUp = function (this)
	gotoHob("../../../lalaurie/HOB_fishing_basket.hob", "round2")
	playSound("/sound/sfx/sfx_zoom_in")
end

---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("lalaurie.pond.fx_flora_basket", false)
end
