---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function(this)
	if isMinigameFinishedBlockedWell() then
		showHint("%那些肯定是幽灵的名字. 没人愿意被遗忘.%")
	else
		showHint("%钟表里写着一个名字 - Nathan.%")
	end
end