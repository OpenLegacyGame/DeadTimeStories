if getAlpha("lalaurie.pond.harrahya_spirit") > 0.0 and getAlpha("lalaurie.pond.harrahya_spirit") < 1.0 then
	setAlpha("lalaurie.pond.harrahya_spirit", 0.0)
end
setAlpha("lalaurie.pond.harrahya_spirit_disappear_fx", 0.0)
stopTreeAnimation("lalaurie.pond.harrahya_spirit_disappear_fx")
rewindTreeAnimation("lalaurie.pond.harrahya_spirit_disappear_fx", 0.0)

if getAlpha("lalaurie.pond.matba_spirit") > 0.0 and getAlpha("lalaurie.pond.matba_spirit") < 1.0 then
	setAlpha("lalaurie.pond.matba_spirit", 0.0)
end
setAlpha("lalaurie.pond.matba_spirit_disappear_fx", 0.0)
stopTreeAnimation("lalaurie.pond.matba_spirit_disappear_fx")
rewindTreeAnimation("lalaurie.pond.matba_spirit_disappear_fx", 0.0)

if getAlpha("lalaurie.pond.matba_spirit") > 0.0 and getAlpha("lalaurie.pond.matba_spirit") < 1.0 then
	setAlpha("lalaurie.pond.matba_spirit", 0.0)
end
setAlpha("lalaurie.pond.selah_spirit_disappear_fx", 0.0)
stopTreeAnimation("lalaurie.pond.selah_spirit_disappear_fx")
rewindTreeAnimation("lalaurie.pond.selah_spirit_disappear_fx", 0.0)

if getAlpha("lalaurie.pond.yaldabaoth_spirit") > 0.0 and getAlpha("lalaurie.pond.yaldabaoth_spirit") < 1.0 then
	setAlpha("lalaurie.pond.yaldabaoth_spirit", 0.0)
end
setAlpha("lalaurie.pond.yaldabaoth_spirit_disappear_fx", 0.0)
stopTreeAnimation("lalaurie.pond.yaldabaoth_spirit_disappear_fx")
rewindTreeAnimation("lalaurie.pond.yaldabaoth_spirit_disappear_fx", 0.0)

if (hasInventoryObject("brush") and 
	hasInventoryObject("spirit_revealing_mixture")) and
	(getAlpha("lalaurie.pond.harrahya_spirit") < 1.0 or
	getAlpha("lalaurie.pond.matba_spirit") < 1.0 or
	getAlpha("lalaurie.pond.selah_spirit") < 1.0 or
	getAlpha("lalaurie.pond.yaldabaoth_spirit") < 1.0) then
	setVisible("lalaurie.pond.fx_reveal_place", true)
	else
	setVisible("lalaurie.pond.fx_reveal_place", false)
end