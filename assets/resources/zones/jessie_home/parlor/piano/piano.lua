---starter
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("jessie_home.parlor.fx_flora_piano", false)
	setVisible("jessie_home.parlor.fx_blink_piano", true)
end
onLMouseUp = function (this)
	showHint("%钢琴的盖锁着. 我很想打开它, 但是我需要钥匙.%")
	gotoState(this, "active")
	playSound("/sound/sfx/sfx_piano_locked")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "piano_key" then
		gotoState(this, "inactive")
		gotoState("jessie_home.parlor.fx_piano_changing", "active")
		gotoState("jessie_home.parlor.piano_open", "unlocked")
		removeInventoryObject("piano_key")
		playSound("/sound/sfx/sfx_key_use")
		stripmineGoodItemAction("piano_key")
	else
		showHint("%钢琴的盖锁着. 我很想打开它, 但是我需要钥匙.%")
		playSound("/sound/sfx/sfx_item_use_fail")
		stripmineWrongItemAction("piano_key")
	end
end	

---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("jessie_home.parlor.fx_flora_piano", false)
	setVisible("jessie_home.parlor.fx_blink_piano", true)
end
onLMouseUp = function (this)
	showHint("%钢琴的盖锁着. 我很想打开它, 但是我需要钥匙.%")
	playSound("/sound/sfx/sfx_piano_locked")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "piano_key" then
		gotoState(this, "inactive")
		gotoState("jessie_home.parlor.fx_piano_changing", "active")
		gotoState("jessie_home.parlor.piano_open", "unlocked")
		removeInventoryObject("piano_key")
		playSound("/sound/sfx/sfx_key_use")
		stripmineGoodItemAction("piano_key")
	end
	if not inventoryObject == "piano_key" then
		showHint("%钢琴的盖锁着. 我很想打开它, 但是我需要钥匙.%")
		playSound("/sound/sfx/sfx_item_use_fail")
		stripmineWrongItemAction("piano_key")
	end
end	
---inactive
onStateEnter = function (this)
	playSound("/sound/sfx/sfx_piano_unlocked")
	setInteractive(this, false)
	setDefaultCursor(this)	
end

