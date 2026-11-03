---active1
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view") 
	setVisible("jessie_home.shed.fx_flora", true)
end
onLMouseUp = function (this)
	gotoHob("../HOB_shed_shelves.hob", "round0")
end
---active2
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view") 
	setVisible("jessie_home.shed.fx_flora", true)
end
onLMouseUp = function (this)
	gotoHob("../HOB_shed_shelves.hob", "round1")
end
---active3
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view") 
	setVisible("jessie_home.shed.fx_flora", true)
end
onLMouseUp = function (this)
	gotoHob("../HOB_shed_shelves.hob", "round2")
end

---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible("jessie_home.shed.fx_flora", false)
end
