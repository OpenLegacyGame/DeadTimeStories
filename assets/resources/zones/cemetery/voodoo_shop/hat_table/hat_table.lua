---top
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
end

---fell
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact")
end

onLMouseUp = function (this)
	gotoState(this, "gone")
	playSound("/sound/sfx/sfx_item_found")
	addInventoryObject("rum")
end

---gone
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
end