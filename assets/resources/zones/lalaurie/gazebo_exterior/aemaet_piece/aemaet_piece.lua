---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	setVisible("lalaurie.gazebo_exterior.fx_blink_aemaet_piece", false)
end
---active
onStateEnter = function (this)
	--gotoState("lalaurie.gazebo_exterior.fx_aemaet_piece", "active") 
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible(this, true) 
	setVisible("lalaurie.gazebo_exterior.fx_blink_aemaet_piece", true) 
end
onLMouseUp = function (this)
	--gotoState("lalaurie.gazebo_exterior.fx_aemaet_piece", "inactive") 
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("aemaet_piece")
	setVisible("lalaurie.gazebo_exterior.fx_blink_aemaet_piece", false)
	if hasInventoryObject("barbathioath_piece") and 
		hasInventoryObject("kedavra_piece") and
		hasInventoryObject("ephphata_piece") and
		hasInventoryObject("aemaet_piece") and
		hasInventoryObject("harrahya_piece") and
		hasInventoryObject("matba_piece") and
		hasInventoryObject("yaldabaoth_piece") and
		hasInventoryObject("selah_piece") then
		dialogText = getString("dialogs/chapter3/jessie_all_spirits_free")
		dialogImage = "/dialogs/talking_face/pt_jessie"
		dialogSound = ""

		showTalkingFaceDialog(dialogText, dialogImage, dialogSound)	
	end
end