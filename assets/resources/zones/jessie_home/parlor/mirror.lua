---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	if isInState("jessie_home.parlor.teacup", "active") then
		showHint("%杰西说我要把光线反射在镜子上. 但是怎么做呢?%")
	end
	if isInState("jessie_home.parlor.teacup", "activeviewed") 
	and not isMinigameFinishedWindowCleaning() then
		showHint("%杰西说我要把光线反射在镜子上. 但是怎么做呢?%")
	end
	if isMinigameFinishedWindowCleaning() then
		gotoCutscene("ch2_end", "/zones/jessie_home/parlor/cs_finish", true)
		gotoState(this, "inactive")
		gotoState("jessie_home.parlor.window", "played")
		gotoState("jessie_home.front_gate.front_gate_carriage", "active")
--		allowZoneAccess("lalaurie")
		gotoState("jessie_home.front_gate.zone_trigger", "c2end")
		setVisible("jessie_home.parlor.fx_mirror", false)
	end
end
---inactive
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	showHint("%日光看上去很不正常, 但它至少能让这个地方看起来欢快点.%")
end