---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function (this)
	showHint("有一个骷髅头不见了. 根据它的 '兄弟们看', 那个骷髅头肯定是彩色的.")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "skull" then
		removeInventoryObject("skull")
		gotoState(this, "ready")
		gotoState("cemetery.voodoo_shop.skulls", "ready")
		showHint("%行了. 我解决了轮盘. 现在我该做什么呢?%")
		playSound("/sound/sfx/sfx_inv_item_put")
		stripmineGoodItemAction("skull")
	else
		showHint("%把它放在轮盘上没有任何帮助!%")
		stripmineWrongItemAction("skull")
	end
end

---ready
onStateEnter = function (this)
	gotoState("cemetery.voodoo_shop.skulls", "ready")
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function (this)
	gotoMinigameSkullsObey()
	playSound("/sound/sfx/sfx_zoom_in")
end

---opened
onStateEnter = function (this)
	gotoState("cemetery.voodoo_shop.skulls", "opened")
	setInteractive(this, true)
	setCursor(this, "view")
	showHint("%一把钥匙!%")
end
onLMouseUp = function (this)
	if not isMinigameFinishedSkullsObey() then
		gotoMinigameSkullsObey()
		playSound("/sound/sfx/sfx_zoom_in")
	else
		showHint("%骷髅头现在都沉默了.%")
	end
end