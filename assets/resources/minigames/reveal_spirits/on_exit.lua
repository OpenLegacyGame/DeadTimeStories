logWarning('barbathioath '..getAlpha("lalaurie.back_yard.barbathioath_spirit"))
logWarning('kedavra '..getAlpha("lalaurie.back_yard.kedavra_spirit"))
logWarning('ephphata '..getAlpha("lalaurie.cellar.ephphata_spirit"))
logWarning('aemaet '..getAlpha("lalaurie.gazebo_exterior.aemaet_spirit"))
logWarning('harrahya '..getAlpha("lalaurie.pond.harrahya_spirit"))
logWarning('matba '..getAlpha("lalaurie.pond.matba_spirit"))
logWarning('selah '..getAlpha("lalaurie.pond.selah_piece"))
logWarning('yaldabaoth '..getAlpha("lalaurie.pond.yaldabaoth_spirit"))

if getAlpha("lalaurie.back_yard.barbathioath_spirit") == 1 and
	getAlpha("lalaurie.back_yard.kedavra_spirit") == 1 then
	
	if not isInState("lalaurie.back_yard.sfx_spirit_whining", "inactive") then
		gotoState("lalaurie.back_yard.sfx_spirit_whining", "inactive")
	end
	setEnabled("lalaurie.back_yard.fx_reveal_place", false)
end

if getAlpha("lalaurie.cellar.ephphata_spirit") == 1 then
	setEnabled("lalaurie.cellar.fx_reveal_place", false)
end

if getAlpha("lalaurie.gazebo_exterior.aemaet_spirit") == 1 then

	if not isInState("lalaurie.gazebo_exterior.sfx_spirit_whining", "inactive") then
		gotoState("lalaurie.gazebo_exterior.sfx_spirit_whining", "inactive")
	end
	setEnabled("lalaurie.gazebo_exterior.fx_reveal_place", false)
end

if 	getAlpha("lalaurie.pond.harrahya_spirit") == 1 and
	getAlpha("lalaurie.pond.matba_spirit") == 1 and
	getAlpha("lalaurie.pond.selah_spirit") == 1 and
	getAlpha("lalaurie.pond.yaldabaoth_spirit") == 1 then

	if not isInState("lalaurie.pond.sfx_spirit_whining", "inactive") then
		gotoState("lalaurie.pond.sfx_spirit_whining", "inactive")
	end
	setEnabled("lalaurie.pond.fx_reveal_place", false)	
end

if getAlpha("lalaurie.back_yard.barbathioath_spirit") == 1 and
	getAlpha("lalaurie.back_yard.kedavra_spirit") == 1 and
	getAlpha("lalaurie.cellar.ephphata_spirit") == 1 and
	getAlpha("lalaurie.gazebo_exterior.aemaet_spirit") == 1 and
	getAlpha("lalaurie.pond.harrahya_spirit") == 1 and
	getAlpha("lalaurie.pond.matba_spirit") == 1 and
	getAlpha("lalaurie.pond.selah_spirit") == 1 and
	getAlpha("lalaurie.pond.yaldabaoth_spirit") == 1 then
	
	logWarning('Reveal Spirits: all spirits revealed!')
	revealSpirits_finish()
	removeAnywhereAppliable("lalaurie.back_yard", "spirit_revealing_mixture")
	removeAnywhereAppliable("lalaurie.cellar", "spirit_revealing_mixture")
	removeAnywhereAppliable("lalaurie.gazebo_exterior", "spirit_revealing_mixture")
	removeAnywhereAppliable("lalaurie.pond", "spirit_revealing_mixture")
	removeInventoryObject("brush")
else
	logWarning('Reveal Spirits: more spirits to reveal')
end