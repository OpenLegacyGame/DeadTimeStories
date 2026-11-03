if not wasInventoryObjectFound("lollipop") and 
	wasInventoryObjectFound("toy_trumpet") then
	gotoState("cajun.mangrove_circle.basket", "active_1")
end

if isInState("cajun.swamp_house.carriage_trigger", "show_hint") then
	showTutorialHint("new_areas", 980,400)
	gotoState("cajun.swamp_house.carriage_trigger", "not_show_hint")
end