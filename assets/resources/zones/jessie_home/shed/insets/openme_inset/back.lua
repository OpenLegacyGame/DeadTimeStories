---active
onStateEnter = function (this)
	setCursor(this, "go_down")
end
onLMouseUp = function(this)
	gotoLocation("jessie_home.shed")
	playSound("/sound/sfx/sfx_zoom_out")
	setVisible("jessie_home.openme_inset.fx_smoke", false)
end

