---active
onStateEnter = function (this)
	setCursor(this, "view") 
	setInteractive(this, true)
	setVisible("lalaurie.pond.fx_blink_scary_tree", true) 
end
onLMouseUp = function (this)
	gotoInSet("lalaurie.scary_tree_inset")
	playSound("/sound/sfx/sfx_zoom_in")
end

---used
onStateEnter = function (this)
	setAlpha(this, 0)
	setVisible("lalaurie.pond.fx_blink_scary_tree", false) 
	setVisible("lalaurie.pond.sound_wood_growling", false)
end
onLMouseUp = function (this)
	showHint("%那棵树好像是活的! 多么邪恶的地方. 肯定有鬼魂.%")
end

