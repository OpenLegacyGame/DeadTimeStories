if	getAlpha("lalaurie.cellar.ephphata_spirit") < 1.0 then

	removeInventoryObject("spirit_revealing_mixture")
	gotoState("lalaurie.cellar.reveal_spell", "with_mixture")
	showHint("%现在, 我来找找幽灵被囚禁的地方!%")
	setVisible("lalaurie.cellar.fx_reveal_place", false)
	playSound("/sound/sfx/sfx_inv_item_put")
	
else
	showHint( getString("puzzles/reveal_spirits/hints/spirits_already_free") )
end