---closed
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	if wasInventoryObjectFound("voodoo_key") then
		showHint("%我想我从骷髅轮盘里拿到的钥匙可以打开这个箱子.%")
	else
		showHint("%箱子锁着. 上面写着: 要想打开巫毒箱子, 得让骷髅头服从!%")
	end
	playSound("/sound/sfx/sfx_drawer_locked")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "voodoo_key" then
		removeInventoryObject("voodoo_key")
		gotoState(this, "opened")
		playSound("/sound/sfx/sfx_key_use")
		stripmineGoodItemAction("voodoo_key")
	else
		showHint("%这个打不开巫毒箱子.%")
		stripmineWrongItemAction("voodoo_key")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end


---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState(this, "opened")
	playSound("/sound/sfx/sfx_box_open")
end

---opened
onStateEnter = function (this)
	gotoState("cemetery.voodoo_shop.chest", "opened")
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function (this)
	gotoState(this, "viewed")
	gotoInSet("cemetery.chest_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---viewed
onStateEnter = function (this)
	enableSpell("curse_of_fire")
	setCursor(this, "view")
end
onLMouseUp = function (this)
	gotoInSet("cemetery.chest_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end