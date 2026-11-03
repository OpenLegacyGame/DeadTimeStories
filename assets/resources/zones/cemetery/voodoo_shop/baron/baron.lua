---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	gotoState("cemetery.voodoo_shop.hat_skull", "top")
	setDefaultCursor(this)
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("cemetery.voodoo_shop.fx_blink_baron", true) 
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	gotoState("cemetery.voodoo_shop.hat_skull", "fell")
	gotoState("cemetery.voodoo_shop.hat_table", "fell")
	playSound("/sound/sfx/sfx_stone_move")
end

---clicked
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("cemetery.voodoo_shop.fx_blink_baron", false) 
end
