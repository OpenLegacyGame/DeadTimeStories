---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("cajun.mangrove_circle.fx_blink_sign", true) 
end
onLMouseUp = function (this)
	if isInState("cajun.mangrove_circle.boat_ready_mask", "clicked") then
		showHint("%我不需要另一个芒果了.%")
	else
		showHint("标牌上写着 '通往芒果树'... 我认为没有船我不可能到那里去.")
	end
end
