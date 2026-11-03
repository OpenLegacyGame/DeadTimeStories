---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	showHint("%那栋房子看上去好像有活人. 那里有人在走动吗?%")
end
---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
end