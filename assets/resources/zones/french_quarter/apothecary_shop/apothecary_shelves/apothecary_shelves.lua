---preactive
onStateEnter = function (this)
	setInteractive(this, true)
end

---active
onStateEnter = function (this)
	setCursor(this, "view") 
	setInteractive(this, true)
	setVisible("french_quarter.apothecary_shop.fx_flora_shelves", true) 
end
onLMouseUp = function (this)
	gotoMinigameSortApothecaryItems( )
	playSound("/sound/sfx/sfx_zoom_in")
end

---used
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, false) 
	setVisible("french_quarter.apothecary_shop.fx_flora_shelves", false) 
	setEnabled("french_quarter.apothecary_shop.fx_ghost_man_static", false)
	setEnabled("french_quarter.apothecary_shop.fx_ghost_man_dynamic", true)
	setTimer(this, 1)
	playSound("/sound/sfx/sfx_rays_appear")
end
onTimer = function (this)
	setEnabled("french_quarter.apothecary_shop.fx_ghost_man_dynamic", false)
end