---safe
onStateEnter = function (this)
	setCursor(this, "view") 
	setTexture(this, "railing_safe")
	setInteractive(this, true)
	setVisible("lalaurie.back_yard.fx_blink_railing", true) 
end
onLMouseUp = function (this)
	gotoInSet("lalaurie.railing_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---cut
onStateEnter = function (this)
	setCursor(this, "view") 
	setTexture(this, "railing_cut")
	setInteractive(this, true)
	setVisible("lalaurie.back_yard.fx_blink_railing", true) 
end
onLMouseUp = function (this)
	gotoInSet("lalaurie.railing_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---used
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setTexture(this, "railing_cut")
end
