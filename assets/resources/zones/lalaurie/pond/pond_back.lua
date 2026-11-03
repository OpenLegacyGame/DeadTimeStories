---active
onStateEnter = function (this)
	setCursor(this, "go_down")
end
onLMouseUp = function (this)
	gotoLocation("lalaurie.gazebo_exterior") 
	playSound("/sound/sfx/sfx_foot_earth")
end