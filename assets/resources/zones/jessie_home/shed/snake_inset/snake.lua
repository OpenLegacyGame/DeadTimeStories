---preactive
onStateEnter = function (this)
	setVisible("jessie_home.shed.fx_blink_snake", false) 
	setVisible("jessie_home.shed.fx_snake_tongue", false) 
	setInteractive(this, false)
	setVisible(this, false)
end

---active
onStateEnter = function (this)
	setVisible("jessie_home.shed.fx_blink_snake", true) 
	setVisible("jessie_home.shed.fx_snake_tongue", true) 
	setCursor(this, "view")
	setInteractive(this, true)
	setVisible(this, true)
	playSound("/sound/sfx/sfx_snake_appears")
end
onLMouseUp = function (this)
	gotoState("jessie_home.snake_inset.inset", "active")
	gotoInSet("jessie_home.snake_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end


---inactive
onStateEnter = function (this)
	setVisible("jessie_home.shed.fx_blink_snake", false) 
	setVisible("jessie_home.shed.fx_snake_tongue", false) 
	setInteractive(this, false)
	setVisible(this, false)
end