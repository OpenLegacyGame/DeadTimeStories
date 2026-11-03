---inactive
onStateEnter = function (this)
	setInteractive(this, false)
end


---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
	--addInventoryObject("mixture_grease_cleaning")
	if isMinigameFinishedCellarDoors() then
		gotoState(this, "open")
	end
end
onLMouseUp = function (this)
	gotoMinigameCellarDoors()
	playSound("/sound/sfx/sfx_zoom_in")
end

---open
onStateEnter = function (this)
	setCursor(this, "go_up")
	setTexture(this, "cellar_door_open")
	setInteractive(this, true)
	playSound("/sound/sfx/sfx_cellar_door_open")
end
onLMouseUp = function (this)
	gotoLocation("lalaurie.cellar") 
	playSound("/sound/sfx/sfx_foot_concrete")
end