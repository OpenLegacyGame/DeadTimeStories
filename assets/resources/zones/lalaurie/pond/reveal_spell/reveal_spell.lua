---no_mixture
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setVisible("lalaurie.pond.reveal_spell_bubbles", false)
	requestParticlesPresim("lalaurie.pond.reveal_spell_bubbles", 2.0)
	setInteractive(this, false)
end

---with_mixture
onStateEnter = function (this)
	setCursor(this, "interact")
	setVisible(this, true)
	setVisible("lalaurie.pond.reveal_spell_bubbles", true)
	setInteractive(this, true)
end

onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "brush" then
		if getAlpha("lalaurie.pond.harrahya_spirit") < 1.0 or
		   getAlpha("lalaurie.pond.matba_spirit") < 1.0 or
		   getAlpha("lalaurie.pond.selah_spirit") < 1.0 or
		   getAlpha("lalaurie.pond.yaldabaoth_spirit") < 1.0 then
			gotoRevealSpiritsMode( "brush", "reveal_spell", "harrahya_spirit", "matba_spirit", "selah_spirit", "yaldabaoth_spirit")
			playSound("/sound/sfx/sfx_m_click_idle")
			stripmineGoodItemAction("brush")
		else
			showHint( getString("puzzles/reveal_spirits/hints/spirits_already_free") )
			stripmineWrongItemAction("brush")
		end
	else
		stripmineWrongItemAction("brush")
	end
end

onLMouseUp = function (this)
	if not isMinigameFinishedRevealSpirits() then
		if getAlpha("lalaurie.pond.harrahya_spirit") < 1.0 or
		   getAlpha("lalaurie.pond.matba_spirit") < 1.0 or
		   getAlpha("lalaurie.pond.selah_spirit") < 1.0 or
		   getAlpha("lalaurie.pond.yaldabaoth_spirit") < 1.0 then
			gotoState(this, "no_mixture")
			setVisible("lalaurie.pond.fx_reveal_place", true)
			playSound("/sound/sfx/sfx_inv_item_pick")
		else
			gotoState(this, "finished")
			setVisible("lalaurie.pond.fx_reveal_place", false)
			playSound("/sound/sfx/sfx_inv_item_pick")
		end
		addInventoryObject("spirit_revealing_mixture")
	else
		showHint( getString("puzzles/reveal_spirits/hints/mixture_not_needed_anymore") )
	end
end

---finished
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setVisible("lalaurie.pond.reveal_spell_bubbles", false)
	requestParticlesPresim("lalaurie.pond.reveal_spell_bubbles", 2.0)
	setInteractive(this, false)
end
