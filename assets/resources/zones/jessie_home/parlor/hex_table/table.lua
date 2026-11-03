---starter
onStateEnter = function (this)
	setInteractive(this, false)
end

---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
	setVisible("jessie_home.parlor.fx_flora_table", true)
	if isMinigameFinishedHexBones() then
		gotoState(this, "inactive")
	end
end
onLMouseUp = function (this)
	gotoMinigameHexBones()
	playSound("/sound/sfx/sfx_zoom_in")
end
---inactive
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
	setVisible("jessie_home.parlor.fx_flora_table", false)
end
onLMouseUp = function (this)
	if isInState("jessie_home.kitchen.lantern", "inactive") then
		showHint("%魔法骨牌似乎没别的要说的了.%")
	else
		gotoMinigameHexBones()
		playSound("/sound/sfx/sfx_zoom_in")
	end
end
