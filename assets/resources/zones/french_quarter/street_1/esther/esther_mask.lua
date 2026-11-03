---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
end
onLMouseUp = function(this)
	if isMinigameFinishedBlockedWell() then
		showHint("%那些名字肯定是幽灵的名字. 没有人希望被遗忘.%")
	else
		showHint("%这里写着一个名字 - Esther.%")
	end
end

---used
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setTexture(this, "gas_lamp_broken")
	setVisible("french_quarter.street_1.fx_blink_gas_lamp", false) 
end