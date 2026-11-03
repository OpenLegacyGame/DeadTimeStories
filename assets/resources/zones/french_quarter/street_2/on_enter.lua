if isInState("french_quarter.street_2.bucket_trigger", "show_hint") then
	showHint("%安东夫人家的钥匙从贝拉的口袋里滑了出来... 而且它现在就在水桶里!%")
	gotoState("french_quarter.street_2.bucket_trigger", "not_show_hint")
end
