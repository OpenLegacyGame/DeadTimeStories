---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible(this, true)
	setVisible("cemetery.voodoo_shop.fx_blink_voodoo_shelf", true)
end
onLMouseUp = function (this)
	gotoInSet("cemetery.tarot_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	setVisible("cemetery.voodoo_shop.fx_blink_voodoo_shelf", false)
end