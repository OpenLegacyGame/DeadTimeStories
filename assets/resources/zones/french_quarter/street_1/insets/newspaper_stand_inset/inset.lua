---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function(this)
	if isMinigameFinishedBlockedWell() then
		showHint("%那些肯定是幽灵的名字. 没人愿意被遗忘.%")
	else
		showHint("%这里写着几个名字 - Joseph, Rainie 和 Alfonse.%")
	end
end