if isInState("french_quarter.street_1.lipstick_trigger", "show_hint") then
	showHint("%她把她的口红扔向了凉亭. 我猜如果我需要用, 我会知道在哪找到它的.%")
	gotoState("french_quarter.street_1.lipstick_trigger", "not_show_hint")
end

if isInState("french_quarter.street_1.carriage_trigger", "show_hint") then
	showTutorialHint("new_areas",880,400)
	gotoState("french_quarter.street_1.carriage_trigger", "not_show_hint")
end

if wasInventoryObjectFound("antidote") then 
	gotoState("french_quarter.morning_room.bag", "inactive")
end