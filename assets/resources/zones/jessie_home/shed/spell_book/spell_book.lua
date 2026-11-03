---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, false)
end
---active
onStateEnter = function (this)
	setCursor(this, "view") 
	setInteractive(this, true)
	setVisible(this, true)
end
onLMouseUp = function (this)
	gotoMinigameOuijiBoard()
	playSound("/sound/sfx/sfx_zoom_in")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	gotoState("jessie_home.shed.drawer4", "closeused")
	gotoState("jessie_home.shed.spellmixing", "active")
	enableSpellBook()
	enableSpell("banish_spirits")
	playSound("/sound/sfx/sfx_item_found")
	showHint("%这是杰西的魔法书!%")
--	gotoState("jessie_home.shed.snake", "active")
end
