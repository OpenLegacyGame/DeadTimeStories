---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	showHint("%这是血! 我需要几滴来制作法术, 但是我用什么来装呢?%")
	playSound("/sound/sfx/sfx_wood_click")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "empty_vial" then
		removeInventoryObject("empty_vial")
		addInventoryObject("vial_with_blood")
		showHint("%瓶子里已经装满了血.%")
		gotoState("lalaurie.pond.scary_tree", "used")
		gotoState("lalaurie.scary_tree_inset.inset", "used")
		gotoState(this, "used")
		playSound("/sound/sfx/sfx_item_found")
		showHint("%我用瓶子装了3滴. 真恶心!%")
		stripmineGoodItemAction("empty_vial")
	else
		showHint("%我不能用这个收集血液.%")
		playSound("/sound/sfx/sfx_item_use_fail")
		stripmineWrongItemAction("empty_vial")
	end
end
---used
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
	playSound("/sound/sfx/sfx_wood_dried")
end
onLMouseUp = function(this)
	showHint("%一滩血迹. 不难相信这周围肯定有鬼魂.%")
end