---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	showHint("%这些植物... 全都如此陌生. 我肯定已经不在堪萨斯了!%")
end
---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
end