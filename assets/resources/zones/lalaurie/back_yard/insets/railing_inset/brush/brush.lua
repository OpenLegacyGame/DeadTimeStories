---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
end

---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setVisible(this, true)
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end

---clicked
onStateEnter = function (this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("brush")
	gotoState("lalaurie.back_yard.railing", "used")
end
