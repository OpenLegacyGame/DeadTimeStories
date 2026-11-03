if getAlpha("lalaurie.gazebo_exterior.aemaet_spirit") > 0.0 and getAlpha("lalaurie.gazebo_exterior.aemaet_spirit") < 1.0 then
	setAlpha("lalaurie.gazebo_exterior.aemaet_spirit", 0.0)
end
setAlpha("lalaurie.gazebo_exterior.aemaet_spirit_disappear_fx", 0.0)
stopTreeAnimation("lalaurie.gazebo_exterior.aemaet_spirit_disappear_fx")
rewindTreeAnimation("lalaurie.gazebo_exterior.aemaet_spirit_disappear_fx", 0.0)

if isInState("lalaurie.gazebo_exterior.spirit_lock", "inactive") then
	gotoCutscene("ch3_end", "/zones/lalaurie/gazebo_exterior/cs_finish", true)
	gotoState("lalaurie.gazebo_exterior.spirit_lock", "cs_played")
	setVisible("lalaurie.gazebo_exterior.fx_miasma01", false)
--	allowZoneAccess("cajun")
end

if hasInventoryObject("brush") and 
	hasInventoryObject("spirit_revealing_mixture") and
	getAlpha("lalaurie.gazebo_exterior.aemaet_spirit") < 1.0 then
	setVisible("lalaurie.gazebo_exterior.fx_reveal_place", true)
	else
	setVisible("lalaurie.gazebo_exterior.fx_reveal_place", false)
end