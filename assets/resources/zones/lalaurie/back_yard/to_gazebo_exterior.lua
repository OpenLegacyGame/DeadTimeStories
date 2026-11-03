---active
onStateEnter = function (this)
	setCursor(this, "go_right")
end
onLMouseUp = function (this)
	gotoLocation("lalaurie.gazebo_exterior") 
	playSound("/sound/sfx/sfx_foot_earth")
end