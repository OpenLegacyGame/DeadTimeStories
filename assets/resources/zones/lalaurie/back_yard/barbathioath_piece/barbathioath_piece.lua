---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	setVisible("lalaurie.back_yard.fx_blink_barbathioath_piece", false)
end
---active
onStateEnter = function (this)
	--gotoState("lalaurie.back_yard.fx_barbathioath_piece", "active") 
	setInteractive(this, true)
	setVisible(this, true) 
	setCursor(this, "interact")
	setVisible("lalaurie.back_yard.fx_blink_barbathioath_piece", true) 
end
onLMouseUp = function (this)
	--gotoState("lalaurie.back_yard.fx_barbathioath_piece", "inactive") 
	gotoState(this, "clicked")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	addInventoryObject("barbathioath_piece")
	setVisible("lalaurie.back_yard.fx_blink_barbathioath_piece", false)
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
		dialogSound = "/sound/dialogues/c3d2_j_you_freed_all_the_spirits"

		showTalkingFaceDialog(dialogText, dialogImage, dialogSound)
	end
end
