---active
onStateEnter = function (this)
	setCursor(this, "go_up")
end
onLMouseUp = function (this)
	gotoLocation("jessie_home.kitchen") 
	playSound("/sound/sfx/sfx_foot_wood")
end
