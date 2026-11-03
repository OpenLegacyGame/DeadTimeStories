---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
end
onLMouseUp = function(this)
	if isMinigameFinishedBlockedWell() then
		showHint("%那些肯定是幽灵的名字. 没人愿意被遗忘.%")
	else
		showHint("%这里写着一个名字 - Elijah.%")
	end
end

---used
onStateEnter = function (this)
	setDefaultCursor(this)
end