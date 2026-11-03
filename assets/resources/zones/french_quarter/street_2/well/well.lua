---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible("french_quarter.street_2.fx_flora_well", false)
	setVisible("french_quarter.street_2.fx_blink_well", true)
end
onLMouseUp = function (this)
	gotoMinigameBlockedWell()
	playSound("/sound/sfx/sfx_zoom_in")
	if not wasMinigameEnteredBlockedWell() then
		showHint("%不要忘记我们? 忘记谁?%")
	end
end

---solved
onStateEnter = function (this)
	setTexture(this, "well_open")
	setInteractive(this, true)
	setCursor(this, "interact")
	setVisible("french_quarter.street_2.fx_flora_well", true)
end
onLMouseUp = function (this)
	gotoState(this, "working")
	showHint("%水桶掉进了井里...%")
end

---working
onStateEnter = function (this)
	setTexture(this, "well_work")
	setInteractive(this, true)
	setCursor(this, "interact")
end
onLMouseUp = function (this)
	gotoState(this, "bucket_full")
	showHint("%水桶装着暗黑色的水出现了...%")
end

---bucket_full
onStateEnter = function (this)
	setTexture(this, "well_open")
	setVisible("french_quarter.street_2.bucket_full", true)
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function (this)
	gotoCutscene("ch5_mid", "/zones/french_quarter/street_2/cs_finish", true)
	gotoState("french_quarter.street_2.well", "bucket_ready")
end

---bucket_ready
onStateEnter = function (this)
	setTexture(this, "well_open")
	setVisible("french_quarter.street_2.bucket_full", true)
	setInteractive(this, true)
	setCursor(this, "view")
end
onLMouseUp = function (this)
	gotoInSet("french_quarter.bucket_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---clicked
onStateEnter = function (this)
	setTexture(this, "well_open")
	setCursor(this, "view")
	setVisible("french_quarter.street_2.fx_blink_well", false)
	setVisible("french_quarter.street_2.fx_flora_well", false)
end
onLMouseUp = function (this)
	showHint("%我从井上移开了幽灵锁. 我想它不会再闹鬼了.%")
end