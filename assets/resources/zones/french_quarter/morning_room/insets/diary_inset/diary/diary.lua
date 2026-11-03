---closed
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setVisible(this, true)
end
onLMouseUp = function (this)
	gotoState(this, "opened")
	playSound("/sound/sfx/sfx_book_open")
end

---opened
onStateEnter = function (this)
	setAlpha(this, 1)
	setTexture(this, "diary_opened")
	setCursor(this, "interact") 
	setInteractive(this, true)
	gotoState("french_quarter.morning_room.diary", "clicked")
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
end
---clicked
onStateEnter = function (this)
--	allowZoneAccess("cemetery")
	gotoState("jessie_home.front_gate.zone_trigger", "c5end")
	gotoState("lalaurie.gazebo_exterior.lipstick", "active")
	dialogCloseScript = "gotoCutscene(\"ch5_end\", \"/zones/french_quarter/morning_room/cs_finish\", true)"
	showMessageDialog("%你读到了安东夫人整个 夏天承受的病痛和可怕的折磨. 派对成为了她最遥远的梦想! 在夏季结束的时候, 她逃回了纽约, 发誓永远不再回来. 新奥尔良对她来说 成为了一个恐怖的地方.%", dialogCloseScript)
	setInteractive(this, false)
	setDefaultCursor(this, true)	
end