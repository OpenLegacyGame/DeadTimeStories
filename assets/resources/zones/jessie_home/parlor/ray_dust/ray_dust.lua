---inactive
onStateEnter = function (this)
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, false)
end
---shine
onStateEnter = function (this)
	playSound("/sound/sfx/sfx_rays_appear")
	setDefaultCursor(this)
	setInteractive(this, false)
	setVisible(this, true)
end
