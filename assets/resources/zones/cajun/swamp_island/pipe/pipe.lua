---inactive
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	showHint("这个骷髅的手握得很紧. 烟斗拿不出来. 肯定有什么办法能让他放开.")
	playSound("/sound/sfx/sfx_item_use_fail")
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("cajun.swamp_island.fx_blink_pipe", true) 
	gotoState("cajun.swamp_island.fx_pipe_changing", "active")
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("pipe")
	setVisible("cajun.swamp_island.fx_blink_pipe", false)
end