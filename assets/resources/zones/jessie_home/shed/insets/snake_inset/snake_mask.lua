---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	gotoState(this, "click1")
	gotoState("jessie_home.snake_inset.inset", "click1")
	playSound("/sound/sfx/sfx_snake_tease")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "apple" then
		showHint("%这条蛇对苹果不感兴趣. 但是如果我把它激怒, 它也许就什么都咬了.%")
		else
		showHint("%这条蛇还没准备好进攻. 但是如果我把它激怒, 它也许就什么都咬了.%")
	end
	setInteractive(this, true)
	setCursor(this, "interact")
	stripmineWrongItemAction(inventoryObject)
	playSound("/sound/sfx/sfx_item_use_fail")
end
---click1
onStateEnter = function (this)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState(this, "click2")
	gotoState("jessie_home.snake_inset.inset", "click2")
	playSound("/sound/sfx/sfx_snake_tease")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "apple" then
		showHint("%这条蛇对苹果不感兴趣. 但是如果我把它激怒, 它也许就什么都咬了.%")
		else
		showHint("%这条蛇还没准备好进攻. 但是如果我把它激怒, 它也许就什么都咬了.%")
	end
	setCursor(this, "interact")
	stripmineWrongItemAction(inventoryObject)
	playSound("/sound/sfx/sfx_item_use_fail")
end
---click2
onStateEnter = function (this)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	showHint("%蛇朝我进攻了! 那样做没什么太大的用处. 也许我可以让它咬别的东西.%")
	gotoState(this, "active")
	gotoState("jessie_home.snake_inset.inset", "active")
	stripmineSnakeBitten()
	playSound("/sound/sfx/sfx_snake_bite")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "apple" then
		removeInventoryObject("apple")
		addInventoryObject("envenomed_apple")
		playSound("/sound/sfx/sfx_item_found")
		gotoState(this, "sleep")
		gotoState("jessie_home.snake_inset.inset", "sleep")
		showHint("%蛇咬了它! 真恶心. 现在这个苹果充满了毒液.%")
		setTimer(this, 3)		
		stripmineGoodItemAction("apple")
	else
		showHint("%蛇朝我进攻了! 那样做没什么太大的用处. 也许我可以让它咬别的东西.%")
		gotoState(this, "active")
		gotoState("jessie_home.snake_inset.inset", "active")
		stripmineSnakeBitten()
		stripmineWrongItemAction("apple")
		playSound("/sound/sfx/sfx_snake_bite")
	end
end
onTimer = function(this)
	gotoLocation("jessie_home.shed")
end

---click4
onStateEnter = function (this)
	setDefaultCursor (this)
end
onLMouseUp = function (this)
	gotoLocation("jessie_home.shed")
	gotoState("jessie_home.shed.snake", "active")
	gotoState("jessie_home.snake_inset.inset", "active")
end
---sleep
onStateEnter = function (this)
	setInteractive(this, true)
	setDefaultCursor (this)
	gotoState("jessie_home.shed.snake", "inactive")
end
