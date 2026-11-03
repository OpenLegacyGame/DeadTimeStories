---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	showHint("%戴尔芬·拉洛瑞和他丈夫的家. 杰西管这里叫 '可怕的地方'. 非常古怪.%")
end
---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
end