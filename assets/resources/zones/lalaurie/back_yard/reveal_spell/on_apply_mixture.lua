if	getAlpha("lalaurie.back_yard.barbathioath_spirit") < 1.0 or
	getAlpha("lalaurie.back_yard.kedavra_spirit") < 1.0 then

	removeInventoryObject("spirit_revealing_mixture")
	gotoState("lalaurie.back_yard.reveal_spell", "with_mixture")
	showHint("%现在, 我要找找幽灵被囚禁在什么地方!%")
	setVisible("lalaurie.back_yard.fx_reveal_place", false)
	playSound("/sound/sfx/sfx_inv_item_put")
	
else
	showHint( getString("puzzles/reveal_spirits/hints/spirits_already_free") )
end