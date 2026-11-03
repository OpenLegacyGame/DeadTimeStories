if	not isMinigameFinishedRevealSpirits() and
	isInState("lalaurie.pond.reveal_spell", "with_mixture") then

	if getAlpha("lalaurie.pond.harrahya_spirit") < 1.0 or
	   getAlpha("lalaurie.pond.matba_spirit") < 1.0 or
	   getAlpha("lalaurie.pond.selah_spirit") < 1.0 or
	   getAlpha("lalaurie.pond.yaldabaoth_spirit") < 1.0 then
		gotoState("lalaurie.pond.reveal_spell", "no_mixture")
		setVisible("lalaurie.pond.fx_reveal_place", true)
	else
		gotoState("lalaurie.pond.reveal_spell", "finished")
		setVisible("lalaurie.pond.fx_reveal_place", false)
	end
	addInventoryObject("spirit_revealing_mixture")
end