---active
onStateEnter = function (this)
	setInteractive (this, true)
	gotoState("jessie_home.piano_inset.matches", "active")
	setTexture(this, "piano_closed")
end
---opened
onStateEnter = function (this)
	setInteractive (this, true)
	gotoState("jessie_home.piano_inset.matches", "inactive")
	setTexture(this, "piano_open")
end