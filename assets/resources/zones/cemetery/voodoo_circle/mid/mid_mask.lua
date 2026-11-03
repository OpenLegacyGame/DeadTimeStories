---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	showHint("%巫毒娃娃应该放在中间.%")
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "voodoo_doll" then
		removeInventoryObject("voodoo_doll")
		setVisible("cemetery.voodoo_circle.voodoo_doll", true)
		setTimer(this, 2)
		setInteractive("cemetery.voodoo_circle.exit", false)
		playSound("/sound/sfx/sfx_rays_appear")
		playAnimation("cemetery.voodoo_circle.fade", "fade")
	else
		showHint("%这不是巫毒娃娃.%")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end
onTimer = function (this)
	gotoCutscene("ch6_end", "/zones/cemetery/voodoo_circle/cs_finish", true)
end
