---inactive
onStateEnter = function (this)
	setVisible(this, false)
end
---active
onStateEnter = function (this)
	setVisible(this, true)
	setTimer(this, 4)
end
onTimer = function(this)
	gotoState(this, "inactive")
	gotoState("jessie_home.bedroom.candles", "inactive")
	showHint("%我需要防风的灯. 蜡烛没用.%")	
end
