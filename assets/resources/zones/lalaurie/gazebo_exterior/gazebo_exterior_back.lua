---active
onStateEnter = function (this)
	setCursor(this, "go_down")
end
onLMouseUp = function (this)
	gotoLocation("lalaurie.back_yard") 
	playSound("/sound/sfx/sfx_foot_earth")
end