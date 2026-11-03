---active
onStateEnter = function (this)
	setCursor(this, "view") 
	setInteractive(this, true)
	setTexture(this, "drawer_03_closed")
end
onLMouseUp = function (this)
	gotoState("jessie_home.openme_inset.inset", "active")
	gotoInSet("jessie_home.openme_inset.inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
---closeinactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setTexture(this, "drawer_03_closed")
end

