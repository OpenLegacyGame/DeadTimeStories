---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function (this)
	showHint("%手套收藏的非常仔细. 肯定是非常昂贵的东西.%")
end

---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
end
