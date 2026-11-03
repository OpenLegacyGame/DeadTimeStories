---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
	setVisible("cemetery.gates.fx_blink_girl_spirit", true)
end
onLMouseUp = function(this)
	showHint("%这个小鬼魂说她的娃娃丢在了沼泽的某个地方, 希望你能找到它.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "baby_doll" then
		removeInventoryObject("baby_doll")
		gotoState(this, "inactive")
		showHint("%小女孩带着她的娃娃飞走了.%")
		stripmineGoodItemAction("baby_doll")
		setEnabled("cemetery.gates.fx_girl_spirit_static", false);
		setEnabled("cemetery.gates.fx_girl_spirit_dynamic", true);
	else
		showHint("%这不是她想要的.%")
		stripmineWrongItemAction("baby_doll")
	end
end

---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, false)
	setVisible("cemetery.gates.fx_blink_girl_spirit", false)
	addInventoryObject("cube_1")
	showHint("%太棒了! 这是大门上装置的一部分.%")
	setTimer(this, 1)
	playSound("/sound/sfx/sfx_rays_appear")
end

onTimer = function(this)
	
	setEnabled("cemetery.gates.fx_girl_spirit_dynamic", false);
end

