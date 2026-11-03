---active
onSpiritRevealed = function (this)
	showHint("%现在幽灵自由了!%")
	setInteractive("lalaurie.gazebo_exterior.aemaet_spirit_mask", true)
	playSound("/sound/sfx/sfx_tip")
end