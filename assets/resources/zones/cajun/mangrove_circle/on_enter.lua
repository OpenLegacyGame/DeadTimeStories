if isInState("cajun.mangrove_circle.mango_trigger", "show_hint") then
	showHint("%The boat brought a mango back with it... I could really use that.%")
	gotoState("cajun.mangrove_circle.mango_trigger", "not_show_hint")
end
