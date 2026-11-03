---inactive
onStateEnter = function (this)
	setInteractive(this, false)
	setDefaultCursor(this)
	setVisible(this, false)
end
---placed
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible(this, true)
	setTexture(this, "perfume_pump")
	showHint("%气泵上写着 '挤压我'.%")
	
end
onLMouseDown = function (this)
	setTexture(this, "pump_sqz")
	playSound("/sound/sfx/sfx_pump_zqueeze")
end
onLMouseUp = function (this)
	if isInState("jessie_home.saint_inset.vial", "placed") then
		gotoState(this, "sqz1")
		gotoState("jessie_home.saint_inset.vial", "1drop")
		gotoState("jessie_home.saint_inset.fx_vial_changing1", "active")
		showHint("%眼泪开始往瓶子里流了.%")
	elseif not isInState("jessie_home.saint_inset.vial", "placed") then
		if math.random() >0.35 then
			showHint("%圣徒流出了眼泪!%")
		else
			showHint("%我需要什么东西来收集眼泪. 我不想浪费它们.%")
		end
		setTexture(this, "perfume_pump")
	end
end
---sqz1
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible(this, true)
	setTexture(this, "perfume_pump")
end
onLMouseDown = function (this)
	setTexture(this, "pump_sqz")
	playSound("/sound/sfx/sfx_pump_zqueeze")
end
onLMouseUp = function (this)
	gotoState(this, "sqz2")
	gotoState("jessie_home.saint_inset.vial", "2drops")
	gotoState("jessie_home.saint_inset.fx_vial_changing2", "active")
	showHint("%眼泪开始往瓶子里流了.%")
end
---sqz2
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible(this, true)
	setTexture(this, "perfume_pump")
end
onLMouseDown = function (this)
	setTexture(this, "pump_sqz")
	playSound("/sound/sfx/sfx_pump_zqueeze")
end
onLMouseUp = function (this)
	showHint("%行了! 我收集到了眼泪.%")
	gotoState(this, "used")
	gotoState("jessie_home.saint_inset.vial", "3drops")
	gotoState("jessie_home.saint_inset.fx_vial_changing3", "active")
end
---used
onStateEnter = function (this)
	setDefaultCursor (this)
	setTexture(this, "perfume_pump")
	--setInteractive(this, false)
end
onLMouseDown = function (this)
	if isInState("jessie_home.saint_inset.vial", "3drops") then
		showHint("%我已经用瓶子收集到了法术需要的足够的眼泪.%")
	end
end
