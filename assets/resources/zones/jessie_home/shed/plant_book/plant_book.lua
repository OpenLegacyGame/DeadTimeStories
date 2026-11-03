---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, false)
end
---active
onStateEnter = function (this)
	setCursor(this, "interact") 
	setInteractive(this, true)
	setVisible(this, true)
end
onLMouseUp = function (this)
	gotoState(this, "clicked")
	showHint("%这是本鉴别植物的书. 这周围生长的都是些特殊的物种, 这本书肯定有用! 我得带上它.%")
	playSound("/sound/sfx/sfx_item_found")
end
---clicked
onStateEnter = function (this)
	setDefaultCursor(this)
	setVisible(this, false)
	setInteractive(this, false)
	enablePlantBook()
	enablePlantBookPage("solomon_seal")
	enablePlantBookPage("devils_shoestring")
	enablePlantBookPage("onion_flower")
	enablePlantBookPage("lily_of_the_valley")
	enablePlantBookPage("orchid")
	enablePlantBookPage("yellow_monkshood")
	showHint("%这是本鉴别植物的书. 我应该拿上它.%")
	if hasInventoryObject("clippers") then
		gotoState("jessie_home.shed.drawer2", "inactive")
	end
	if hasInventoryObject("clippers") then
		if hasInventoryObject("red_string") then
			if not hasInventoryObject("devils_shoestring") then
				gotoState("jessie_home.front_gate.to_HOB_plants", "active2")
			end
		end
	end
end
