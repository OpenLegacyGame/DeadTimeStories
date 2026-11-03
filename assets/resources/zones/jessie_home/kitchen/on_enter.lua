if isInState("jessie_home.kitchen.garlic_trigger", "show_hint") then
	showHint("%房子震动时大蒜掉在了地止. 也许我应该捡起来.%")
	gotoState("jessie_home.kitchen.garlic_trigger", "not_show_hint")
end
