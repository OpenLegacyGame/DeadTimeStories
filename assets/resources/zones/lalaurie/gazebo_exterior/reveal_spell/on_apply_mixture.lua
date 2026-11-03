if	getAlpha("lalaurie.gazebo_exterior.aemaet_spirit") < 1.0 then

	removeInventoryObject("spirit_revealing_mixture")
	gotoState("lalaurie.gazebo_exterior.reveal_spell", "with_mixture")
	showHint("%现在, 我来找找幽灵被囚禁在哪里!%")
	setVisible("lalaurie.gazebo_exterior.fx_reveal_place", false)
	playSound("/sound/sfx/sfx_inv_item_put")
	
else
	showHint( getString("puzzles/reveal_spirits/hints/spirits_already_free") )
end	