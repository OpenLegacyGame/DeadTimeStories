---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("lalaurie.gazebo_exterior.fx_blink_spirit_lock", true) 
end
onLMouseUp = function (this)
	gotoMinigameSpiritLock()
	playSound("/sound/sfx/sfx_zoom_in")
end

---inactive
onStateEnter = function (this)
	setDefaultCursor(this, true)
	setInteractive(this, false)
	setVisible(this, false)
	setVisible("lalaurie.gazebo_exterior.fx_blink_spirit_lock", false) 
	gotoState("lalaurie.gazebo_exterior.gazebo_door", "unlocked")
end

---cs_played