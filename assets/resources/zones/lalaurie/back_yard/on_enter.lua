if getAlpha("lalaurie.back_yard.barbathioath_spirit") > 0.0 and getAlpha("lalaurie.back_yard.barbathioath_spirit") < 1.0 then
	setAlpha("lalaurie.back_yard.barbathioath_spirit", 0.0)
end
setAlpha("lalaurie.back_yard.barbathioath_spirit_disappear_fx", 0.0)
stopTreeAnimation("lalaurie.back_yard.barbathioath_spirit_disappear_fx")
rewindTreeAnimation("lalaurie.back_yard.barbathioath_spirit_disappear_fx", 0.0)

if getAlpha("lalaurie.back_yard.kedavra_spirit") > 0.0 and getAlpha("lalaurie.back_yard.kedavra_spirit") < 1.0 then
	setAlpha("lalaurie.back_yard.kedavra_spirit", 0.0)
end
setAlpha("lalaurie.back_yard.kedavra_spirit_disappear_fx", 0.0)
stopTreeAnimation("lalaurie.back_yard.kedavra_spirit_disappear_fx")
rewindTreeAnimation("lalaurie.back_yard.kedavra_spirit_disappear_fx", 0.0)

if (hasInventoryObject("brush") and 
	hasInventoryObject("spirit_revealing_mixture")) and
	(getAlpha("lalaurie.back_yard.barbathioath_spirit") < 1.0 or
	getAlpha("lalaurie.back_yard.kedavra_spirit") < 1.0) then
	setVisible("lalaurie.back_yard.fx_reveal_place", true)
	else
	setVisible("lalaurie.back_yard.fx_reveal_place", false)
end


if (hasInventoryObject("brush") and 
	hasInventoryObject("spirit_revealing_mixture") and
	isInState("lalaurie.back_yard.reveal_hint_trigger", "active"))  then
	showHint("%我已经有了幽灵显形法术! 魔法书上说 '把混合剂涂抹在空气中' 找到幽灵. 我想我可以猜出大概的位置.%")
	gotoState("lalaurie.back_yard.reveal_hint_trigger", "triggered")
end
if (not hasInventoryObject("brush") and 
	hasInventoryObject("spirit_revealing_mixture") and
	isInState("lalaurie.back_yard.brush_hint_trigger", "active"))  then
	showHint("%我已经有了幽灵显形法术, 但是我还需要一支画笔.%")
	gotoState("lalaurie.back_yard.brush_hint_trigger", "triggered")
end

if isInState("lalaurie.back_yard.carriage_trigger", "show_hint") then
	showTutorialHint("new_areas",80,245)
	gotoState("lalaurie.back_yard.carriage_trigger", "not_show_hint")
end