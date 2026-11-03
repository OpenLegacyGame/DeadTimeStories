---active1
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.front_gate.fx_flora", true)
end
onLMouseUp = function (this)
	gotoHob("../HOB_plants.hob", "round0")
	playSound("/sound/sfx/sfx_zoom_in")
end
---active2
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.front_gate.fx_flora", true)
end
onLMouseUp = function (this)
	gotoHob("../HOB_plants.hob", "round1")
	playSound("/sound/sfx/sfx_zoom_in")
end
---active3
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.front_gate.fx_flora", true)
end
onLMouseUp = function (this)
	gotoHob("../HOB_plants.hob", "round2")
	playSound("/sound/sfx/sfx_zoom_in")
end
---active4
onStateEnter = function (this)
	setVisible(this, true)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.front_gate.fx_flora", true)
end
onLMouseUp = function (this)
	gotoHob("../HOB_plants.hob", "round3")
	playSound("/sound/sfx/sfx_zoom_in")
end
---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
	setDefaultCursor(this)
	setVisible("jessie_home.front_gate.fx_flora", false)
	if hasInventoryObject("red_string") and
		isPlantBookEnabled() and
		hasInventoryObject("clippers") and
		not hasInventoryObject("devils_shoestring") then
		gotoState(this, "active2")
	end
end


	
	
--	if hasInventoryObject("red_string") then
--		if isPlantBookEnabled() then
--			if hasInventoryObject("clippers") then
--				if not hasInventoryObject("devils_shoestring") then
--					gotoState(this, "active2")
--				end
--			end
--		end
--	end
