---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("cemetery.voodoo_shop.fx_blink_cat", true) 
end
onLMouseUp = function (this)
	gotoState(this, "roaring")
	showHint("%当心你的手! 猫着点咬到你.%")
	playSound("/sound/sfx/sfx_cat_reaction")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "fish" then
		removeInventoryObject("fish")
		gotoState(this, "inactive")
		gotoState("cemetery.voodoo_shop.cat_eating", "eating")
		gotoState("cemetery.voodoo_shop.baron", "active")
		showHint("%过来, 小猫咪...%")
		stripmineGoodItemAction("fish")
		playSound("/sound/sfx/sfx_cat_small")
	else
		showHint("%我不能用这个抓住那只蓝色的猫.%")
		stripmineWrongItemAction("fish")
	end
end

---roaring
onStateEnter = function (this)
	setTexture(this, "cat_roaring")
	setInteractive(this, false)
	setTimer(this, 1)
end

onTimer = function(this)
	gotoState(this, "active")	
	setTexture(this, "cat")
end

---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, false)
	setVisible("cemetery.voodoo_shop.fx_blink_cat", false) 
end