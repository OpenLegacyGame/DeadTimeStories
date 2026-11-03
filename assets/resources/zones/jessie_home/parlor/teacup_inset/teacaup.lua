---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.parlor.fx_blink_teacup", true)
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.teacup_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
---activeviewed
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.parlor.fx_blink_teacup", false)
end
onLMouseUp = function (this)
	gotoInSet("jessie_home.teacup_inset")
	gotoState("jessie_home.teacup_inset.inset", "activeviewed")
end
---used
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("jessie_home.parlor.fx_blink_teacup", false)
end
