---no_mixture
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setVisible("lalaurie.back_yard.reveal_spell_bubbles", false)
	requestParticlesPresim("lalaurie.back_yard.reveal_spell_bubbles", 2.0)
	setInteractive(this, false)
end

---with_mixture
onStateEnter = function (this)
	setCursor(this, "interact")
	setVisible(this, true)
	setVisible("lalaurie.back_yard.reveal_spell_bubbles", true)
	setInteractive(this, true)
	if not hasInventoryObject("brush") then
	showHint("%The spellbook says to brush the paint through the air. But I need to find a paint brush.%")
	end
end

onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "brush" then
		if getAlpha("lalaurie.back_yard.barbathioath_spirit") < 1.0 or getAlpha("lalaurie.back_yard.kedavra_spirit") < 1.0 then
			gotoRevealSpiritsMode( "brush", "reveal_spell", "barbathioath_spirit", "kedavra_spirit")
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
		if getAlpha("lalaurie.back_yard.barbathioath_spirit") < 1.0 or 
			getAlpha("lalaurie.back_yard.kedavra_spirit") < 1.0 then
			gotoState(this, "no_mixture")
			setVisible("lalaurie.back_yard.fx_reveal_place", true)
			playSound("/sound/sfx/sfx_inv_item_pick")
		else
			gotoState(this, "finished")
			setVisible("lalaurie.back_yard.fx_reveal_place", false)
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
	setVisible("lalaurie.back_yard.reveal_spell_bubbles", false)
	requestParticlesPresim("lalaurie.back_yard.reveal_spell_bubbles", 2.0)
	setInteractive(this, false)
end
