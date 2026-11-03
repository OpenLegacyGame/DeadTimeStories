---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function(this)
	if isMinigameFinishedBlockedWell() then
		showHint("%那些名字肯定是幽灵的名字. 没有人愿意被遗忘.%")
	else
		showHint("%有个名字写在那里. 是 Elfi.%")
	end
end