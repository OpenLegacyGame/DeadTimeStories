---active
onStateEnter = function (this)
	setCursor(this, "go_up")
end
onLMouseUp = function (this)
	gotoLocation("lalaurie.pond") 
	playSound("/sound/sfx/sfx_foot_earth")
end