---closed
onStateEnter = function (this)
	setTexture(this, "black_book_closed")
	setCursor(this, "interact") 
	setInteractive(this, true)
	setVisible(this, true)
end
onLMouseUp = function (this)
	gotoState(this, "opened")
end

---opened
onStateEnter = function (this)
	setTexture(this, "black_book_opened")
	setCursor(this, "interact") 
	setInteractive(this, true)
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
end
---clicked
onStateEnter = function (this)
	showMessageDialog("%火咒已经增加到了魔法书里.%")
	setInteractive(this, false)
	setDefaultCursor(this, true)	
end