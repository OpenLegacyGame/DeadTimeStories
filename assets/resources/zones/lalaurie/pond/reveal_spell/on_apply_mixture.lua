if	getAlpha("lalaurie.pond.harrahya_spirit") < 1.0 or
	getAlpha("lalaurie.pond.matba_spirit") < 1.0 or
	getAlpha("lalaurie.pond.selah_spirit") < 1.0 or
	getAlpha("lalaurie.pond.yaldabaoth_spirit") < 1.0 then

	removeInventoryObject("spirit_revealing_mixture")
	gotoState("lalaurie.pond.reveal_spell", "with_mixture")
	showHint("%现在, 我来找找幽灵被囚禁在什么地方!%")
	setVisible("lalaurie.pond.fx_reveal_place", false)
	playSound("/sound/sfx/sfx_inv_item_put")
	
else
	showHint( getString("puzzles/reveal_spirits/hints/spirits_already_free") )
end