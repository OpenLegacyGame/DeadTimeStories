---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.bedroom.fx_flora_bedside_table", true)
end
onLMouseUp = function (this)
	gotoHob("../../../jessie_home/HOB_bedside_table.hob", "round0")
end

---active_1
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("jessie_home.bedroom.fx_flora_bedside_table", true)
end
onLMouseUp = function (this)
	gotoHob("../../../jessie_home/HOB_bedside_table.hob", "round1")
end


---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible("jessie_home.bedroom.fx_flora_bedside_table", false)
end
