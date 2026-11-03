if	not isMinigameFinishedRevealSpirits() and
	isInState("lalaurie.gazebo_exterior.reveal_spell", "with_mixture") then

	if getAlpha("lalaurie.gazebo_exterior.aemaet_spirit") < 1.0 then
		gotoState("lalaurie.gazebo_exterior.reveal_spell", "no_mixture")
		setVisible("lalaurie.gazebo_exterior.fx_reveal_place", true)
	else
		gotoState("lalaurie.gazebo_exterior.reveal_spell", "finished")
		setVisible("lalaurie.gazebo_exterior.fx_reveal_place", false)
	end
	addInventoryObject("spirit_revealing_mixture")
end