---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
end
onLMouseUp = function(this)
	if hasInventoryObject("doll_without_head") then
		showHint("%我可以使用那个娃娃头, 但是没有帮助我不可能够到它.%")
	else
		showHint("%那个娃娃头看上去令人毛骨悚然. 它是某种图腾吗?%")
	end
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "boat_hook" then
		if hasInventoryObject("doll_without_head") then
			removeInventoryObject("doll_without_head")
			removeInventoryObject("boat_hook")
			addInventoryObject("baby_doll")
			gotoState(this, "used")
			showMessageDialog("%你找到了娃娃丢失的头. 现在娃娃已经完成了.%")
		else
			addInventoryObject("doll_head")
			removeInventoryObject("boat_hook")
			gotoState(this, "used")
		end
		stripmineGoodItemAction("boat_hook")
		playSound("/sound/sfx/sfx_item_found")
	else
		showHint("%I can't reach the doll head with that.%")
		stripmineWrongItemAction("boat_hook")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end

---used
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, false)
	setVisible("cajun.swamp_house.fx_blink_doll_head", false) 
end





