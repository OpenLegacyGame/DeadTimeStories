---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setVisible(this, false)
	setDefaultCursor(this, true)
end

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setVisible(this, true)
	setCursor(this, "interact") 
end
onLMouseUp = function (this)
	showHint("%现在, 我可以到芒果树上摘些水果了!%")
	gotoCutscene("ch4_mid", "/zones/cajun/mangrove_circle/cs_finish", true)
	gotoState(this, "clicked")
	gotoState("cajun.mangrove_circle.boat_ready", "clicked")
	gotoState("cajun.mangrove_circle.mango", "active")	
end

---clicked
onStateEnter = function (this)
	setInteractive(this, true)
end

onLMouseUp = function (this)
	showHint("%我不需要把船再划出去了.%")
end