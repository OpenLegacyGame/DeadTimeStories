---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible("jessie_home.shed.fx_flora_spell", false)
end

---active
onStateEnter = function (this)
	setCursor(this, "view")
	setVisible("jessie_home.shed.fx_flora_spell", true)
	if isMinigameFinishedBanishSpirits() then
		gotoState(this, "inactive")
	end
end
onLMouseUp = function (this)
	gotoMinigameBanishSpirits()
	playSound("/sound/sfx/sfx_zoom_in")
end
---active2
onStateEnter = function (this)
	setCursor(this, "view")
	setVisible("jessie_home.shed.fx_flora_spell", true)
	if isMinigameFinishedGraveyardGrease() then
		gotoState(this, "inactive")
	end
end
onLMouseUp = function (this)
	gotoMinigameGraveyardGrease()
	playSound("/sound/sfx/sfx_zoom_in")
end

---active3
onStateEnter = function (this)
	setCursor(this, "view")
	setVisible("jessie_home.shed.fx_flora_spell", true)
	if isMinigameFinishedSpiritRevealingMixture() then
		gotoState(this, "inactive")
	end
end
onLMouseUp = function (this)
	gotoMinigameSpiritRevealingMixture()
	playSound("/sound/sfx/sfx_zoom_in")
end

---active4
onStateEnter = function (this)
	setCursor(this, "view")
	setVisible("jessie_home.shed.fx_flora_spell", true)
	if isMinigameFinishedAconiteAntidote() then
		gotoState(this, "inactive")
	end
end
onLMouseUp = function (this)
	gotoMinigameAconiteAntidote()
	playSound("/sound/sfx/sfx_zoom_in")
end

---active5
onStateEnter = function (this)
	setCursor(this, "view")
	setVisible("jessie_home.shed.fx_flora_spell", true)
	if isMinigameFinishedVoodooDoll() then
		gotoState(this, "inactive")
	end
end
onLMouseUp = function (this)
	gotoMinigameVoodooDoll() 
	playSound("/sound/sfx/sfx_zoom_in")
end
