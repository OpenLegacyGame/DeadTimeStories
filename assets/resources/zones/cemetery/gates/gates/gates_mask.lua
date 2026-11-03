---closed
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setVisible("cemetery.gates.fx_blink_gates", true) 
end
onLMouseUp = function (this)
	showHint("%门上还缺一些方块. 在我弄清门上说的是什么之前, 我得先找到方块.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "cube_1" then
		if isInState("cemetery.gates.right_block", "full") then
			gotoState(this, "assembled")
			else
			showHint("%太好了! 这个方块好像很合适, 但是我还需要另外一块.%")	
		end
		gotoState("cemetery.gates.left_block", "full")
		removeInventoryObject("cube_1")
		playSound("/sound/sfx/sfx_inv_item_put")
	end
	if inventoryObject == "cube_2" then
		if isInState("cemetery.gates.left_block", "full") then
			gotoState(this, "assembled")
			else
			showHint("%太好了! 这个方块好像很合适, 但是我还需要另外一块.%")	
		end
		gotoState("cemetery.gates.right_block", "full")
		removeInventoryObject("cube_2")
		playSound("/sound/sfx/sfx_inv_item_put")
	end
end
---assembled
onStateEnter = function (this)
	setCursor(this, "view") 
	showHint("%现在, 方块已经放好了, 我可以打开门锁... 怎么开呢...%")
end
onLMouseUp = function (this)
	gotoMinigameCemeteryGate()
	playSound("/sound/sfx/sfx_zoom_in")
end

---opened
onStateEnter = function (this)
	gotoState("cemetery.gates.left_block", "open")
	gotoState("cemetery.gates.right_block", "open")
	setCursor(this, "go_up") 
	setTexture("cemetery.gates.gates", "gates_opened")
	setInteractive(this, true)
	setVisible("cemetery.gates.fx_blink_gates", false) 
	setVisible("cemetery.gates.fx_skull_eyes", false) 
end
onLMouseUp = function (this)
	if not isMinigameFinishedVoodooCircle() then
		gotoMinigameVoodooCircle()
	else
		gotoLocation("cemetery.voodoo_circle")
	end
	playSound("/sound/sfx/sfx_foot_earth")
	gotoState("lalaurie.pond.fishing_basket", "active_2")
	if not wasInventoryObjectFound("string") then
		gotoState("jessie_home.bedroom.bedside_table", "active_1")
	end
end
