---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setTexture(this, "drawer_04_closed")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "key" then
		gotoState(this, "open")
		removeInventoryObject("key")
		playSound("/sound/sfx/sfx_key_use")
		setVisible("jessie_home.openme_inset.fx_blink_drawer", false)
		stripmineGoodItemAction("key")
	else
		showHint("%这个抽屉锁着. 也许钥匙就在这周围.%")
		stripmineWrongItemAction("key")
	end
end
onLMouseUp = function (this)
	showHint("%这个抽屉锁着. 也许钥匙就在这周围.%")
	playSound("/sound/sfx/sfx_drawer_locked")
	gotoState(this, "active2")
end
---active2
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setTexture(this, "drawer_04_closed")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "key" then
		gotoState(this, "open")
		removeInventoryObject("key")
		playSound("/sound/sfx/sfx_key_use")
		setVisible("jessie_home.openme_inset.fx_blink_drawer", false)
		stripmineGoodItemAction("key")
	else
		showHint("%这个抽屉锁着. 也许钥匙就在这周围.%")
		stripmineWrongItemAction("key")
	end
end
onLMouseUp = function (this)
	showHint("%这个抽屉锁着. 也许钥匙就在这周围.%")
	playSound("/sound/sfx/sfx_drawer_locked")
end
---open
onStateEnter = function (this)
	setDefaultCursor(this)
	setTexture(this, "drawer_04_opened")
	playSound("/sound/sfx/sfx_drawer_open")
	gotoState("jessie_home.shed.spell_book", "active")
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState(this, "close")
	playSound("/sound/sfx/sfx_drawer_close")
	if not isSpellBookEnabled() then
		gotoState(this, "unlocked")
	end
end
---close
onStateEnter = function (this)
	setDefaultCursor(this)
	gotoState("jessie_home.shed.spell_book", "inactive")
	gotoState(this, "active")
	setTexture(this, "drawer_04_closed")
end
---closeused
onStateEnter = function (this)
	setCursor(this, "interact")
	setTexture(this, "drawer_04_opened")
end
onLMouseUp = function (this)
	setDefaultCursor(this)
	gotoState(this, "inactive")
end
---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setTexture(this, "drawer_04_closed")
end
---unlocked
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setTexture(this, "drawer_04_closed")
end
onLMouseUp = function (this)
	gotoState(this, "open")
	playSound("/sound/sfx/sfx_drawer_open")
end
