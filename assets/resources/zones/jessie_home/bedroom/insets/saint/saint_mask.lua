---active
onStateEnter = function (this)
	setCursor(this, "interact")
	setInteractive (this, true)
	
end
onLMouseUp = function(this)
	if isInState("jessie_home.saint_inset.pump", "inactive") then
		showHint("%在圣徒上有一个阀门. 安装什么用的?%")
	end

end

onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "perfume_pump" then
		
		gotoState("jessie_home.saint_inset.pump", "placed")
		removeInventoryObject("perfume_pump")
		if isInState("jessie_home.saint_inset.vial", "placed") then
			gotoState(this, "cry")
			gotoState("jessie_home.saint_inset.inset", "cry")
		end
		stripmineGoodItemAction("perfume_pump")
	elseif inventoryObject == "empty_vial" then
		gotoState("jessie_home.saint_inset.vial", "placed")
		removeInventoryObject("empty_vial")
		if isInState("jessie_home.saint_inset.pump", "placed") then
			gotoState(this, "cry")
			gotoState("jessie_home.saint_inset.inset", "cry")
		end
		stripmineGoodItemAction("empty_vial")
	elseif inventoryObject ~= "perfume_pump" 
		and not isInState("jessie_home.saint_inset.pump", "placed")
		and inventoryObject ~= "empty_vial" then
		showHint("%这个不适合安在圣徒的阀门上.%")
		stripmineWrongItemAction("perfume_pump")
	elseif inventoryObject ~= "empty_vial"
		and isInState("jessie_home.saint_inset.pump", "placed") 
		and not isInState("jessie_home.saint_inset.vial", "placed") then
		showHint("%我不能用这个来收集眼泪.%")
		stripmineWrongItemAction("empty_vial")
	end	
end
---cry
onStateEnter = function (this)
	setDefaultCursor (this)
	setInteractive (this, true)
	if isInState("jessie_home.saint_inset.vial", "3drops") then
		gotoState(this, "inactive")
		gotoState("jessie_home.saint_inset.inset", "inactive")
	end
end
onLMouseUp = function(this)
	showHint("%收集眼泪用的所有东西都装好了.%")
	if isInState("jessie_home.saint_inset.vial", "3drops") then
		gotoState(this, "inactive")
		gotoState("jessie_home.saint_inset.inset", "inactive")
		showHint("%好了! 我收集到了眼泪.%")
	end
end
---inactive
onStateEnter = function (this)
	setDefaultCursor (this)
end
