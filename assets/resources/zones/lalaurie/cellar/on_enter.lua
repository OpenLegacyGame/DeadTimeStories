if getAlpha("lalaurie.cellar.ephphata_spirit") > 0.0 and getAlpha("lalaurie.cellar.ephphata_spirit") < 1.0 then
	setAlpha("lalaurie.cellar.ephphata_spirit", 0.0)
end
setAlpha("lalaurie.cellar.ephphata_spirit_disappear_fx", 0.0)
stopTreeAnimation("lalaurie.cellar.ephphata_spirit_disappear_fx")
rewindTreeAnimation("lalaurie.cellar.ephphata_spirit_disappear_fx", 0.0)

if hasInventoryObject("brush") and 
	hasInventoryObject("spirit_revealing_mixture") and
	getAlpha("lalaurie.cellar.ephphata_spirit") < 1.0 then
	setVisible("lalaurie.cellar.fx_reveal_place", true)
	else
	setVisible("lalaurie.cellar.fx_reveal_place", false)
end

if not isMinigameFinishedRats() and
	isInState("lalaurie.cellar.rats_trigger", "trigger_0") then
	gotoMinigameRats()
	gotoState("lalaurie.cellar.rats_trigger", "trigger_1")
	else 
	if isInState("lalaurie.cellar.rats_trigger", "trigger_1") and not
		isMinigameFinishedRats() then
		gotoState("lalaurie.cellar.rats_trigger", "trigger_0")
		gotoLocation("lalaurie.back_yard")
	end
end