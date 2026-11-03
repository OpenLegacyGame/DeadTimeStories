---active
onStateEnter = function (this)
	setInteractive(this, true)
	setCursor(this, "view")
	setVisible(this, true)
end
onLMouseUp = function (this)
	showHint("%这幅画的背面写着 '卡津族人是如此的邪恶, 他们现在像恶魔一样出没于沼泽!'%")
end