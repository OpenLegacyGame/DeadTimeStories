---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	if isPlantBookEnabled() then
		showHint("%我不需要干花.%")
	else
		showHint("%干花. 杰西肯定是个植物学家.%")
	end
end
---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
end