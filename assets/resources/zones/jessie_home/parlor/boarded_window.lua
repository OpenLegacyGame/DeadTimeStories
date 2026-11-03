---active
onStateEnter = function (this)
	setCursor(this, "view")
	setInteractive(this, true)
end
onLMouseUp = function (this)
	if (hasInventoryObject("jug_of_water") and
		hasInventoryObject("lemon") and
		hasInventoryObject("white_pepper") and
		hasInventoryObject("tears")) then
		showHint("%I have found everything I can in Jessie’s house, at least for now. I need to look around outside to keep moving forward.%")
	else
		showHint("%That window is boarded up tight.%")
	end
end
---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
end