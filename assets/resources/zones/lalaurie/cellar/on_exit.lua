if	not isMinigameFinishedRevealSpirits() and
	isInState("lalaurie.cellar.reveal_spell", "with_mixture") then
	
	if getAlpha("lalaurie.cellar.ephphata_spirit") < 1.0 then
		gotoState("lalaurie.cellar.reveal_spell", "no_mixture")
		setVisible("lalaurie.cellar.fx_reveal_place", true)
	else
		gotoState("lalaurie.cellar.reveal_spell", "finished")
		setVisible("lalaurie.cellar.fx_reveal_place", false)
	end
	addInventoryObject("spirit_revealing_mixture")
end