---active
onStateEnter = function (this)
	setCursor(this, "go_down")
end
onLMouseUp = function(this)
	gotoLocation("cemetery.voodoo_shop")
	playSound("/sound/sfx/sfx_zoom_out")
end

