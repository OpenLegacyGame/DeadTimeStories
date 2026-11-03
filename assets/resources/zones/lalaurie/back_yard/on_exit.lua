if	not isMinigameFinishedRevealSpirits() and
	isInState("lalaurie.back_yard.reveal_spell", "with_mixture") then
	
	if getAlpha("lalaurie.back_yard.barbathioath_spirit") < 1.0 or 
		getAlpha("lalaurie.back_yard.kedavra_spirit") < 1.0 then
		gotoState("lalaurie.back_yard.reveal_spell", "no_mixture")
		setVisible("lalaurie.back_yard.fx_reveal_place", true)
	else
		gotoState("lalaurie.back_yard.reveal_spell", "finished")
		setVisible("lalaurie.back_yard.fx_reveal_place", false)
	end
	addInventoryObject("spirit_revealing_mixture")
end