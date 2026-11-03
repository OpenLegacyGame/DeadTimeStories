---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
	setVisible("cemetery.gates.fx_blink_boy_spirit", true)
end
onLMouseUp = function(this)
	showHint("%这个小鬼魂说他的球丢在了沼泽的某个地方, 希望你能找到它.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "ball" then
		removeInventoryObject("ball")
		gotoState("cemetery.gates.slingshot", "active")
		gotoState(this, "inactive")
		showHint("%男孩飞走了, 留下了他的弹弓.%")
		stripmineGoodItemAction("ball")
		setEnabled("cemetery.gates.fx_boy_spirit_static", false);
		setEnabled("cemetery.gates.fx_boy_spirit_dynamic", true);
	else
		showHint("%这不是他想要的.%")
		stripmineWrongItemAction("ball")
	end
end

---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, false)
	setVisible("cemetery.gates.fx_blink_boy_spirit", false)
	addInventoryObject("cube_2")
	showHint("%太棒了! 这是大门上装置的一部分.%")
	setTimer(this, 1)
	playSound("/sound/sfx/sfx_rays_appear")
end

onTimer = function(this)
	setEnabled("cemetery.gates.fx_boy_spirit_dynamic", false);
end