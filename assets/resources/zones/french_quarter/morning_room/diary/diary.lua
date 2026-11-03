---inactive
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("french_quarter.morning_room.fx_blink_diary", true) 
end
onLMouseUp = function(this)
	dialogText = getString("dialogs/chapter5/anton1")
	dialogImage = "/dialogs/talking_face/pt_anton"
	dialogSound = "/sound/dialogues/c5d5_ma_weeping"
	scriptAfterDialog = ''

	showTalkingFaceDialog(dialogText, dialogImage, dialogSound, scriptAfterDialog)
end
onInventoryApplied = function (this, inventoryObject)
	if inventoryObject == "antidote" then	
		removeInventoryObject("antidote")
		gotoState(this, "dialogue")	
		playSound("/sound/sfx/mg/mg_finish")
	else
		showHint("%这不是解毒剂.%")
		stripmineWrongItemAction("antidote")
		playSound("/sound/sfx/sfx_item_use_fail")
	end
end

---dialogue
onStateEnter = function(this)
	dialogText = getString("dialogs/chapter5/anton2")
	dialogImage = "/dialogs/talking_face/pt_anton"
	dialogSound = "/sound/dialogues/c5d6_ma_im_free"
	scriptAfterDialog = 'gotoState("french_quarter.morning_room.diary", "wait")'

	showTalkingFaceDialog(dialogText, dialogImage, dialogSound, scriptAfterDialog)
end
---wait
onStateEnter = function(this)
	setEnabled("french_quarter.morning_room.fx_diary_static", false)
	setEnabled("french_quarter.morning_room.fx_diary_dynamic", true)
	setEnabled("french_quarter.morning_room.fx_ghost_man_static", false)
	setEnabled("french_quarter.morning_room.fx_ghost_woman_static", false)
	setEnabled("french_quarter.morning_room.fx_ghost_man_dynamic", true)
	setEnabled("french_quarter.morning_room.fx_ghost_woman_dynamic", true)
	setTimer(this, 1)
	playSound("/sound/sfx/sfx_rays_appear")
end

onTimer = function(this)
	setEnabled("french_quarter.morning_room.fx_diary_dynamic", false)
	setEnabled("french_quarter.morning_room.fx_ghost_man_dynamic", false)
	setEnabled("french_quarter.morning_room.fx_ghost_woman_dynamic", false)	
	
	stripmineGoodItemAction("antidote")
	gotoState(this, "active")
end


---active
onStateEnter = function (this)
	setTexture(this, "diary_cure")
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function (this)
	gotoInSet("french_quarter.diary_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end


---clicked
onStateEnter = function (this)
	setTexture(this, "diary_open")
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function (this)
	gotoInSet("french_quarter.diary_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end
