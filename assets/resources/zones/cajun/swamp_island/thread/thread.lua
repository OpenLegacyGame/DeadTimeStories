---inactive
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	setDefaultCursor(this, true)
	setVisible("cajun.swamp_island.fx_blink_thread", false)
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
	setVisible("cajun.swamp_island.fx_blink_thread", true) 
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
	if hasInventoryObject("needle") then
		removeInventoryObject("needle")
		addInventoryObject("sewing_kit")
		showMessageDialog("%你已经把针和线结合成了缝纫工具.%")
	else
		addInventoryObject("thread")
	end
	setVisible("cajun.swamp_island.fx_blink_thread", false)
end