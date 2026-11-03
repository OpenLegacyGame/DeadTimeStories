--Chapter 1 hints
--finding spell book
if isInState("jessie_home.shed.drawer4", "active") then
	enableLocationHint("hints/hint_group_general/hint0/text")
end
if wasInventoryObjectFound("key") and not
	hasInventoryObject("key") then
	disableLocationHint("hints/hint_group_general/hint0/text")
end
if isInState("jessie_home.shed.drawer4", "open") then
	enableLocationHint("hints/hint_group_general/hint1/text")
end
if isMinigameFinishedWithoutReward_OuijiBoard() then
	disableLocationHint("hints/hint_group_general/hint1/text")
	enableLocationHint("hints/hint_group_general/hint2/text")
end
if isSpellBookEnabled() then
	disableLocationHint("hints/hint_group_general/hint0/text")
	disableLocationHint("hints/hint_group_general/hint1/text")
	disableLocationHint("hints/hint_group_general/hint2/text")
end
--ingredients
if isSpellBookEnabled() and 
	not wasInventoryObjectFound("graveyard_ash") then
	enableLocationHint("hints/hint_group_ingredients_spell/hint0/text")
end
if isSpellBookEnabled() and 
	not wasInventoryObjectFound("holy_water") then
	enableLocationHint("hints/hint_group_ingredients_spell/hint1/text")
end
if isSpellBookEnabled() and 
	not wasInventoryObjectFound("salt") then
	enableLocationHint("hints/hint_group_ingredients_spell/hint2/text")
end
if isSpellBookEnabled() and 
	not wasInventoryObjectFound("mint") then
	enableLocationHint("hints/hint_group_ingredients_spell/hint3/text")
end
if isSpellBookEnabled() and 
	not wasInventoryObjectFound("clove") then
	enableLocationHint("hints/hint_group_ingredients_spell/hint4/text")
end
if isSpellBookEnabled() and 
	not wasInventoryObjectFound("devils_shoestring") then
	enableLocationHint("hints/hint_group_ingredients_spell/hint5/text")
end
if isSpellBookEnabled() and 
	not wasInventoryObjectFound("solomon_seal") then
	enableLocationHint("hints/hint_group_ingredients_spell/hint6/text")
end
if not isInState("jessie_home.front_gate.sprouts", "active") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint6/text")
end
if isInState("jessie_home.front_gate.sprouts", "watered") then
	enableLocationHint("hints/hint_group_ingredients_spell/hint7/text")
end
if isSpellBookEnabled() and 
	not wasInventoryObjectFound("envenomed_apple") then
	enableLocationHint("hints/hint_group_ingredients_spell/hint8/text")
end
if isSpellBookEnabled() and 
	not wasInventoryObjectFound("bottle") then
	enableLocationHint("hints/hint_group_ingredients_spell/hint9/text")
end
if isSpellBookEnabled() and 
	not wasInventoryObjectFound("red_string") then
	enableLocationHint("hints/hint_group_ingredients_spell/hint10/text")
end

if wasInventoryObjectFound("graveyard_ash") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint0/text")
end
if wasInventoryObjectFound("holy_water") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint1/text")
end
if wasInventoryObjectFound("salt") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint2/text")
end
if wasInventoryObjectFound("mint") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint3/text")
end
if wasInventoryObjectFound("clove") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint4/text")
end
if wasInventoryObjectFound("devils_shoestring") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint5/text")
end
if wasInventoryObjectFound("solomon_seal") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint6/text")
end
if wasInventoryObjectFound("solomon_seal") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint7/text")
end
if wasInventoryObjectFound("envenomed_apple") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint8/text")
end
if wasInventoryObjectFound("bottle") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint9/text")
end
if wasInventoryObjectFound("red_string") then
	disableLocationHint("hints/hint_group_ingredients_spell/hint10/text")
end
--mixing
if (wasInventoryObjectFound("graveyard_ash") and
	wasInventoryObjectFound("holy_water") and
	wasInventoryObjectFound("salt") and
	wasInventoryObjectFound("mint") and
	wasInventoryObjectFound("clove") and
	wasInventoryObjectFound("devils_shoestring") and
	wasInventoryObjectFound("solomon_seal") and
	wasInventoryObjectFound("envenomed_apple") and
	wasInventoryObjectFound("red_string") and
	not isMinigameFinishedBanishSpirits()) then
	enableLocationHint("hints/hint_group_mixing_spell/hint1/text")
end
if (isIngredientUsedBanishSpirits("graveyard_ash") and
	isIngredientUsedBanishSpirits("holy_water") and
	isIngredientUsedBanishSpirits("salt") and
	isIngredientUsedBanishSpirits("mint") and
	isIngredientUsedBanishSpirits("clove") and
	isIngredientUsedBanishSpirits("devils_shoestring") and
	isIngredientUsedBanishSpirits("solomon_seal") and
	isIngredientUsedBanishSpirits("envenomed_apple") and
	not hasInventoryObject("bottle")) then
	disableLocationHint("hints/hint_group_mixing_spell/hint1/text")
	enableLocationHint("hints/hint_group_mixing_spell/hint2/text")
	enableLocationHint("hints/hint_group_mixing_spell/hint3/text")
end
if isMixedBanishSpirits() then
	disableLocationHint("hints/hint_group_mixing_spell/hint2/text")
end
if (wasInventoryObjectFound("red_string") and
	not hasInventoryObject("red_string")) then
	disableLocationHint("hints/hint_group_mixing_spell/hint2/text")
end
if wasInventoryObjectFound("mixture_banish_spirits") then
	disableLocationHint("hints/hint_group_mixing_spell/hint3/text")
end
--using amulet
if isMinigameFinishedBanishSpirits() then
	enableLocationHint("hints/hint_group_using_amulet/hint1/text")
	disableLocationHint("hints/hint_group_ingredients_spell/hint0/text")
	disableLocationHint("hints/hint_group_ingredients_spell/hint1/text")
	disableLocationHint("hints/hint_group_ingredients_spell/hint2/text")
	disableLocationHint("hints/hint_group_ingredients_spell/hint3/text")
	disableLocationHint("hints/hint_group_ingredients_spell/hint4/text")
	disableLocationHint("hints/hint_group_ingredients_spell/hint5/text")
	disableLocationHint("hints/hint_group_ingredients_spell/hint6/text")
	disableLocationHint("hints/hint_group_ingredients_spell/hint7/text")
	disableLocationHint("hints/hint_group_ingredients_spell/hint8/text")
	disableLocationHint("hints/hint_group_ingredients_spell/hint9/text")
	disableLocationHint("hints/hint_group_ingredients_spell/hint9/text")
end
if isInState("jessie_home.front_gate.miazma", "inactive") then
	disableLocationHint("hints/hint_group_using_amulet/hint1/text")
end
--cleaning hints
if isInState("jessie_home.front_gate.miazma", "inactive") then
	disableLocationHint("hints/hint_group_general/hint0/text")
	disableLocationHint("hints/hint_group_general/hint1/text")
	disableLocationHint("hints/hint_group_general/hint2/text")
	disableLocationHint("hints/hint_group_mixing_spell/hint1/text")
	disableLocationHint("hints/hint_group_mixing_spell/hint2/text")
	disableLocationHint("hints/hint_group_mixing_spell/hint3/text")
	disableLocationHint("hints/hint_group_using_amulet/hint1/text")	
end	
	
--chapter2
--what to do
if isInState("jessie_home.parlor.hex_table", "active") then
	enableLocationHint("hints/hint_group_what_to_do/hint1/text")
	enableLocationHint("hints/hint_group_what_to_do/hint2/text")
end
if isMinigameFinishedHexBones() then
	disableLocationHint("hints/hint_group_what_to_do/hint1/text")
end
if isInState("jessie_home.parlor.piano", "active") or 
	wasMinigameEnteredPiano() then
	disableLocationHint("hints/hint_group_what_to_do/hint2/text")
end
--hexbones
if isMinigameStartedHexBones() then
	enableLocationHint("hints/hint_group_hex_bones/hint1/text")
end
if isMinigameStartedHexBones() and
	isLocationHintUsed("hints/hint_group_hex_bones/hint1/text") then
	enableLocationHint("hints/hint_group_hex_bones/hint2/text")
end

if isMinigameFinishedHexBones() then
	disableLocationHint("hints/hint_group_hex_bones/hint1/text")
end
if isMinigameFinishedHexBones() then
	disableLocationHint("hints/hint_group_hex_bones/hint2/text")
end
--mirror
if isInState("jessie_home.parlor.hex_table", "active") then
	enableLocationHint("hints/hint_group_unlbocking_mirror/hint1/text")
end
if wasMinigameEnteredWindowCleaning() then
	disableLocationHint("hints/hint_group_unlbocking_mirror/hint1/text")
	enableLocationHint("hints/hint_group_unlbocking_mirror/hint2/text")
end
if isSpellEnabled("graveyard_grease") then
	disableLocationHint("hints/hint_group_unlbocking_mirror/hint2/text")
end
if isMinigameFinishedWindowCleaning() then
	disableLocationHint("hints/hint_group_unlbocking_mirror/hint2/text")
end

--infredients
if isSpellEnabled("graveyard_grease") and
	not wasInventoryObjectFound("lemon") then
	enableLocationHint("hints/hint_group_grease_ingredients/hint1/text")
end
if isSpellEnabled("graveyard_grease") and 
	not wasInventoryObjectFound("tears") then
	enableLocationHint("hints/hint_group_grease_ingredients/hint2/text")
end	
if isSpellEnabled("graveyard_grease") and 
	not wasInventoryObjectFound("onion_flower") then
	enableLocationHint("hints/hint_group_grease_ingredients/hint4/text")
end	
if isSpellEnabled("graveyard_grease") and 
	not wasInventoryObjectFound("garlic_head") then
	enableLocationHint("hints/hint_group_grease_ingredients/hint5/text")
end	
if isSpellEnabled("graveyard_grease") and 
	not wasInventoryObjectFound("white_pepper") then
	enableLocationHint("hints/hint_group_grease_ingredients/hint6/text")
end	
if isSpellEnabled("graveyard_grease") and 
	not wasInventoryObjectFound("jug_of_water") then
	enableLocationHint("hints/hint_group_grease_ingredients/hint7/text")
end	
if isSpellEnabled("graveyard_grease") and 
	not wasInventoryObjectFound("bowl") then
	enableLocationHint("hints/hint_group_grease_ingredients/hint8/text")
end	
if (wasInventoryObjectFound("perfume_pump") and
	not hasInventoryObject("perfume_pump")) then
	enableLocationHint("hints/hint_group_grease_ingredients/hint3/text")
	disableLocationHint("hints/hint_group_grease_ingredients/hint2/text")
end
if wasInventoryObjectFound("lemon") then
	disableLocationHint("hints/hint_group_grease_ingredients/hint1/text")
end
if wasInventoryObjectFound("tears") then
	disableLocationHint("hints/hint_group_grease_ingredients/hint3/text")
end
if wasInventoryObjectFound("onion_flower") then
	disableLocationHint("hints/hint_group_grease_ingredients/hint4/text")
end
if wasInventoryObjectFound("garlic_head") then
	disableLocationHint("hints/hint_group_grease_ingredients/hint5/text")
end
if wasInventoryObjectFound("white_pepper") then
	disableLocationHint("hints/hint_group_grease_ingredients/hint6/text")
end
if wasInventoryObjectFound("jug_of_water") then
	disableLocationHint("hints/hint_group_grease_ingredients/hint7/text")
end
if wasInventoryObjectFound("bowl") then
	disableLocationHint("hints/hint_group_grease_ingredients/hint8/text")
end
--mixing
if (wasInventoryObjectFound("lemon") and 
	wasInventoryObjectFound("tears") and 
	wasInventoryObjectFound("onion_flower") and 
	wasInventoryObjectFound("garlic_head") and 
	wasInventoryObjectFound("white_pepper") and 
	wasInventoryObjectFound("jug_of_water") and
	not isMinigameFinishedGraveyardGrease()) then
	enableLocationHint("hints/hint_group_mixing_grease/hint1/text")
end
if (isIngredientUsedGraveyardGrease("lemon") and
	isIngredientUsedGraveyardGrease("tears") and
	isIngredientUsedGraveyardGrease("onion_flower") and
	isIngredientUsedGraveyardGrease("garlic_head") and
	isIngredientUsedGraveyardGrease("white_pepper") and
	isIngredientUsedGraveyardGrease("jug_of_water") and
	not hasInventoryObject("bowl") and
	not isMinigameFinishedGraveyardGrease()) then
	enableLocationHint("hints/hint_group_mixing_grease/hint2/text")
	disableLocationHint("hints/hint_group_mixing_grease/hint1/text")
end	
if (isIngredientUsedGraveyardGrease("lemon") and
	isIngredientUsedGraveyardGrease("tears") and
	isIngredientUsedGraveyardGrease("onion_flower") and
	isIngredientUsedGraveyardGrease("garlic_head") and
	isIngredientUsedGraveyardGrease("white_pepper") and
	isIngredientUsedGraveyardGrease("jug_of_water") and
	hasInventoryObject("empty_vial") and
	not hasInventoryObject("bowl")) then
	enableLocationHint("hints/hint_group_mixing_grease/hint3/text")
	disableLocationHint("hints/hint_group_mixing_grease/hint2/text")
end	
if isMinigameFinishedGraveyardGrease() then
	disableLocationHint("hints/hint_group_mixing_grease/hint1/text")
	disableLocationHint("hints/hint_group_mixing_grease/hint2/text")
	disableLocationHint("hints/hint_group_mixing_grease/hint3/text")
end
--using grease cleaning mixture
if isMinigameFinishedGraveyardGrease() then
	enableLocationHint("hints/hint_group_using_mixture/hint1/text")
end
if wasInventoryObjectFound("sponge") then
	disableLocationHint("hints/hint_group_using_mixture/hint1/text")
	enableLocationHint("hints/hint_group_using_mixture/hint2/text")
end
if isMinigameFinishedWindowCleaning() then
	disableLocationHint("hints/hint_group_using_mixture/hint2/text")
end
--lighting a spark
if isMinigameFinishedHexBones() then
	enableLocationHint("hints/hint_group_lighting_spark/hint1/text")
	enableLocationHint("hints/hint_group_lighting_spark/hint2/text")
	enableLocationHint("hints/hint_group_lighting_spark/hint3/text")
end
if wasInventoryObjectFound("wick") and 
	wasInventoryObjectFound("oil") and
	not hasInventoryObject("wick") and
	not hasInventoryObject("oil") then
	disableLocationHint("hints/hint_group_lighting_spark/hint1/text")
	enableLocationHint("hints/hint_group_lighting_spark/hint5/text")
end
if wasInventoryObjectFound("matches") then
	disableLocationHint("hints/hint_group_lighting_spark/hint2/text")
end
if wasInventoryObjectFound("wick") and 
	not hasInventoryObject("wick") then
	disableLocationHint("hints/hint_group_lighting_spark/hint3/text")
end
if isLocationHintUsed("hints/hint_group_lighting_spark/hint3/text") or
	hasInventoryObject("wick") or
	isInState("jessie_home.kitchen.lantern_wick", "visible") then
	enableLocationHint("hints/hint_group_lighting_spark/hint4/text")
end
if wasInventoryObjectFound("oil") and 
	not hasInventoryObject("oil") then
	disableLocationHint("hints/hint_group_lighting_spark/hint4/text")
end
if isInState("jessie_home.kitchen.lantern", "inactive") then
	disableLocationHint("hints/hint_group_lighting_spark/hint5/text")
end
--piano
if isInState("jessie_home.parlor.piano", "active") then
	enableLocationHint("hints/hint_group_playing_piano/hint1/text")
end
if isInState("jessie_home.parlor.piano_open", "unlocked") then
	disableLocationHint("hints/hint_group_playing_piano/hint1/text")
	enableLocationHint("hints/hint_group_playing_piano/hint2/text")
end
if isMinigameFinishedPiano() then
	disableLocationHint("hints/hint_group_playing_piano/hint1/text")
	disableLocationHint("hints/hint_group_playing_piano/hint2/text")
end




--Chapter 3 hints

	--cellar door
if isInState("lalaurie.back_yard.cellar_door", "active") then 
	enableLocationHint("hints/hint_group_opening_the_cellar/hint1/text")
end	
if wasInventoryObjectFound("ash") and not hasInventoryObject("ash") then
	enableLocationHint("hints/hint_group_opening_the_cellar/hint2/text")
	disableLocationHint("hints/hint_group_opening_the_cellar/hint1/text")
end
if isMinigameFinishedCellarDoors() then
	disableLocationHint("hints/hint_group_opening_the_cellar/hint2/text")
end
	--dove cote
if wasMinigameEnteredRats() then
	enableLocationHint("hints/hint_group_dove_cote/hint1/text")
end
if wasInventoryObjectFound("seeds") then 
	disableLocationHint("hints/hint_group_dove_cote/hint1/text")
	enableLocationHint("hints/hint_group_dove_cote/hint2/text")
end
if wasInventoryObjectFound("rope") then 
	disableLocationHint("hints/hint_group_dove_cote/hint2/text")
end
if (wasInventoryObjectFound("seeds") and 
	wasInventoryObjectFound("rope")) then
	enableLocationHint("hints/hint_group_dove_cote/hint3/text")
end
if isMinigameFinishedDoveCote() then
	disableLocationHint("hints/hint_group_dove_cote/hint3/text")
end
	--freeing the spirits
if isInState("lalaurie.back_yard.cellar_door", "active") then 
	enableLocationHint("hints/hint_group_freeng_the_spirits/hint1/text")
end
if isSpellEnabled("spirit_revealing_mixture") then
	disableLocationHint("hints/hint_group_freeng_the_spirits/hint1/text")
end	
	--finding ingredients
if isSpellEnabled("spirit_revealing_mixture") then
	enableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint1/text")
	enableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint2/text")
	enableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint3/text")
	enableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint4/text")
	enableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint6/text")
end
if isInState("lalaurie.pond.fishing_basket", "active") or 
	hasInventoryObject("metal_cutters") then
	enableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint5/text")
end
if wasInventoryObjectFound("lily") then 
	disableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint1/text")
end
if wasInventoryObjectFound("dirt") then 
	disableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint2/text")
end
if wasInventoryObjectFound("sulfur") then 
	disableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint3/text")
end
if wasInventoryObjectFound("vial_with_blood") then 
	disableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint4/text")
end
if wasInventoryObjectFound("white_paint") then 
	disableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint5/text")
end
if wasInventoryObjectFound("feather") then 
	disableLocationHint("hints/hint_group_finding_reagents_for_reveal_spirits/hint6/text")
end
	--mixing
if (wasInventoryObjectFound("lily") and
	wasInventoryObjectFound("dirt") and
	wasInventoryObjectFound("sulfur") and
	wasInventoryObjectFound("vial_with_blood") and
	wasInventoryObjectFound("white_paint") and
	wasInventoryObjectFound("feather")) then
	enableLocationHint("hints/hint_group_mixing_reveal_spirits/hint1/text")
end
if isLoaFullSpiritRevealingMixture() then
	disableLocationHint("hints/hint_group_mixing_reveal_spirits/hint1/text")
	enableLocationHint("hints/hint_group_mixing_reveal_spirits/hint2/text")
end
if isMixedSpiritRevealingMixture() then	
	disableLocationHint("hints/hint_group_mixing_reveal_spirits/hint2/text")
	enableLocationHint("hints/hint_group_mixing_reveal_spirits/hint3/text")
end
if wasInventoryObjectFound("spirit_revealing_mixture") then
	disableLocationHint("hints/hint_group_mixing_reveal_spirits/hint2/text")
end
if (isIngredientUsedSpiritRevealingMixture("lily") and
	isIngredientUsedSpiritRevealingMixture("dirt") and
	isIngredientUsedSpiritRevealingMixture("sulfur") and
	isIngredientUsedSpiritRevealingMixture("vial_with_blood") and
	isIngredientUsedSpiritRevealingMixture("white_paint") and
	isIngredientUsedSpiritRevealingMixture("feather")) then
	disableLocationHint("hints/hint_group_mixing_reveal_spirits/hint1/text")
--	disableLocationHint("hints/hint_group_mixing_reveal_spirits/hint2/text")
end
if wasInventoryObjectFound("spirit_revealing_mixture") then 
	disableLocationHint("hints/hint_group_mixing_reveal_spirits/hint3/text")
end
	--using spell
if isMinigameFinishedSpiritRevealingMixture() then
	enableLocationHint("hints/hint_group_using_reveal_spirits/hint1/text")
end
if wasInventoryObjectFound("brush") then 
	disableLocationHint("hints/hint_group_using_reveal_spirits/hint1/text")
end
if (wasInventoryObjectFound("spirit_revealing_mixture") and
	wasInventoryObjectFound("brush")) then
	enableLocationHint("hints/hint_group_using_reveal_spirits/hint2/text")
end
if getAlpha("lalaurie.back_yard.barbathioath_spirit") == 1 or
	getAlpha("lalaurie.back_yard.kedavra_spirit") == 1 or
	getAlpha("lalaurie.cellar.ephphata_spirit") == 1 or
	getAlpha("lalaurie.gazebo_exterior.aemaet_spirit") == 1 or
	getAlpha("lalaurie.pond.harrahya_spirit") == 1 or
	getAlpha("lalaurie.pond.matba_spirit") == 1 or
	getAlpha("lalaurie.pond.selah_spirit") == 1 or
	getAlpha("lalaurie.pond.yaldabaoth_spirit") == 1 then
	disableLocationHint("hints/hint_group_using_reveal_spirits/hint2/text")
	enableLocationHint("hints/hint_group_using_reveal_spirits/hint3/text")
	enableLocationHint("hints/hint_group_using_reveal_spirits/hint4/text")
	enableLocationHint("hints/hint_group_using_reveal_spirits/hint5/text")
	enableLocationHint("hints/hint_group_using_reveal_spirits/hint6/text")
end
if isInState("lalaurie.pond.reveal_spell", "finished") or isMinigameFinishedRevealSpirits() then
	disableLocationHint("hints/hint_group_using_reveal_spirits/hint3/text")
end
if isInState("lalaurie.back_yard.reveal_spell", "finished") or isMinigameFinishedRevealSpirits() then
	disableLocationHint("hints/hint_group_using_reveal_spirits/hint4/text")
end
if isInState("lalaurie.gazebo_exterior.reveal_spell", "finished") or isMinigameFinishedRevealSpirits() then
	disableLocationHint("hints/hint_group_using_reveal_spirits/hint5/text")
end
if isInState("lalaurie.cellar.reveal_spell", "finished") or isMinigameFinishedRevealSpirits() then
	disableLocationHint("hints/hint_group_using_reveal_spirits/hint6/text")
end
	--unblocking gazebo
if isMinigameFinishedRevealSpirits() then
	enableLocationHint("hints/hint_group_unlocking_gazebo/hint1/text")	
end
if isMinigameFinishedSpiritLock() then
	disableLocationHint("hints/hint_group_unlocking_gazebo/hint1/text")
end

--Chapter 4 hints

--Getting the Blue Anole
if isInState("cajun.mangrove_circle.anole_mask", "preactive") then
	enableLocationHint("hints/hint_group_getting_anole/hint1/text")	
end
if isInState("cajun.mangrove_circle.anole_mask", "active") or
	wasInventoryObjectFound("blue_anole") then
	disableLocationHint("hints/hint_group_getting_anole/hint1/text")
end
if isInState("cajun.mangrove_circle.anole_mask", "active") and not 
	hasInventoryObject("mango")	then
	enableLocationHint("hints/hint_group_getting_anole/hint2/text")
end
if wasInventoryObjectFound("blue_anole") then
	disableLocationHint("hints/hint_group_getting_anole/hint2/text")
end
if hasInventoryObject("mango") then
	enableLocationHint("hints/hint_group_getting_anole/hint3/text")
end
if wasInventoryObjectFound("blue_anole") then
	disableLocationHint("hints/hint_group_getting_anole/hint3/text")
end
if hasInventoryObject("blue_anole") and
	isInState("cajun.swamp_house.house", "active") then
	enableLocationHint("hints/hint_group_getting_anole/hint4/text")
end
if isInState("cajun.house_inset.anole_place", "clicked") then
	disableLocationHint("hints/hint_group_getting_anole/hint4/text")	
end
if hasInventoryObject("blue_anole") and not
	isInState("cajun.swamp_house.house", "active") then
	enableLocationHint("hints/hint_group_getting_anole/hint5/text")
end
if isInState("cajun.house_inset.anole_place", "clicked") then
	disableLocationHint("hints/hint_group_getting_anole/hint5/text")	
end
--Fixing the boat's sail
if wasMinigameEnteredSail() and not
	isMinigameFinishedSail() then
	enableLocationHint("hints/hint_group_fixing_the_sail/hint1/text")
end
if wasInventoryObjectFound("bloomers") and
	wasInventoryObjectFound("nightdress") and
	wasInventoryObjectFound("shirt") and
	wasInventoryObjectFound("trowel") then
	disableLocationHint("hints/hint_group_fixing_the_sail/hint1/text")
end
if (wasInventoryObjectFound("bloomers") and
	wasInventoryObjectFound("nightdress") and
	wasInventoryObjectFound("shirt") and
	wasInventoryObjectFound("trowel")) then
	enableLocationHint("hints/hint_group_fixing_the_sail/hint2/text")
end
if wasInventoryObjectFound("orange_1") and 
	wasInventoryObjectFound("orange_2") and 
	wasInventoryObjectFound("orange_3") then
	disableLocationHint("hints/hint_group_fixing_the_sail/hint2/text")
end
if (hasInventoryObject("orange_1") and
	hasInventoryObject("orange_2") and
	hasInventoryObject("orange_3")) and not
	isMinigameFinishedLeeches() then
	enableLocationHint("hints/hint_group_fixing_the_sail/hint3/text")
end
 if isMinigameFinishedWithoutReward_Leeches() and not
	wasInventoryObjectFound("tablecloth") then
	enableLocationHint("hints/hint_group_fixing_the_sail/hint4/text")
end
if wasInventoryObjectFound("tablecloth") then
	disableLocationHint("hints/hint_group_fixing_the_sail/hint3/text")
	disableLocationHint("hints/hint_group_fixing_the_sail/hint4/text")
 end
if (wasInventoryObjectFound("bloomers") and
	wasInventoryObjectFound("nightdress") and
	wasInventoryObjectFound("shirt") and
	wasInventoryObjectFound("trowel") and
	wasInventoryObjectFound("tablecloth")) then
	enableLocationHint("hints/hint_group_fixing_the_sail/hint5/text")
end
if sail_isFabricFit() then
	disableLocationHint("hints/hint_group_fixing_the_sail/hint5/text")
	enableLocationHint("hints/hint_group_fixing_the_sail/hint6/text")
end
if wasInventoryObjectFound("sewing_kit") then
	disableLocationHint("hints/hint_group_fixing_the_sail/hint6/text")
end
if hasInventoryObject("sewing_kit") then
	enableLocationHint("hints/hint_group_fixing_the_sail/hint7/text")
end
if isMinigameFinishedSail() then
	disableLocationHint("hints/hint_group_fixing_the_sail/hint7/text")
	disableLocationHint("hints/hint_group_fixing_the_sail/hint5/text")
	disableLocationHint("hints/hint_group_fixing_the_sail/hint6/text")
end
--Getting rid of the alligators
if isInState("cajun.mangrove_circle.picture", "preactive") then
	enableLocationHint("hints/hint_group_alligators/hint1/text")
end
if isInState("cajun.mangrove_circle.picture", "active") then
	disableLocationHint("hints/hint_group_alligators/hint1/text")
	enableLocationHint("hints/hint_group_alligators/hint2/text")
	enableLocationHint("hints/hint_group_alligators/hint3/text")
	enableLocationHint("hints/hint_group_alligators/hint4/text")
	enableLocationHint("hints/hint_group_alligators/hint5/text")
	enableLocationHint("hints/hint_group_alligators/hint6/text")
	enableLocationHint("hints/hint_group_alligators/hint7/text")
end
if isInState("cajun.swamp_house.alligator_4", "inactive") then
	disableLocationHint("hints/hint_group_alligators/hint2/text")
end
if isInState("cajun.swamp_house.alligator_6", "inactive") then
	disableLocationHint("hints/hint_group_alligators/hint3/text")
end
if isInState("cajun.swamp_house.alligator_5", "inactive") then
	disableLocationHint("hints/hint_group_alligators/hint4/text")
end
if isInState("cajun.swamp_house.alligator_3", "inactive") then
	disableLocationHint("hints/hint_group_alligators/hint5/text")
end
if isInState("cajun.swamp_house.alligator_2", "inactive") then
	disableLocationHint("hints/hint_group_alligators/hint6/text")
end
if isInState("cajun.swamp_house.alligator_1", "inactive") then
	disableLocationHint("hints/hint_group_alligators/hint7/text")
end
if (isInState("cajun.swamp_house.alligator_1", "inactive") and
	isInState("cajun.swamp_house.alligator_2", "inactive") and
	isInState("cajun.swamp_house.alligator_3", "inactive") and
	isInState("cajun.swamp_house.alligator_4", "inactive") and
	isInState("cajun.swamp_house.alligator_5", "inactive") and
	isInState("cajun.swamp_house.alligator_6", "inactive")) then
	disableLocationHint("hints/hint_group_alligators/hint1/text")
end

--Chapter 5 hints

--start
if isInState("french_quarter.street_2.hint_trigger", "triggered") then
	enableLocationHint("hints/hint_group_french_quarter/hint1/text")
end
if isInState("french_quarter.street_2.hint_trigger", "disabled") then
	disableLocationHint("hints/hint_group_french_quarter/hint1/text")
end
if isInState("french_quarter.apothecary_shop.apothecary_shelves", "active") then
	enableLocationHint("hints/hint_group_french_quarter/hint2/text")
end
if wasMinigameEnteredSortApothecaryItems() then
	disableLocationHint("hints/hint_group_french_quarter/hint2/text")
end
--Unblocking the well
if isInState("french_quarter.street_2.hint_trigger", "disabled") then
	enableLocationHint("hints/hint_group_unblocking_the_well/hint1/text")
end
if wasMinigameEnteredBlockedWell() then
	disableLocationHint("hints/hint_group_unblocking_the_well/hint1/text")
	enableLocationHint("hints/hint_group_unblocking_the_well/hint2/text")
	enableLocationHint("hints/hint_group_unblocking_the_well/hint3/text")
	enableLocationHint("hints/hint_group_unblocking_the_well/hint4/text")
	enableLocationHint("hints/hint_group_unblocking_the_well/hint5/text")
	enableLocationHint("hints/hint_group_unblocking_the_well/hint6/text")
	enableLocationHint("hints/hint_group_unblocking_the_well/hint7/text")
	enableLocationHint("hints/hint_group_unblocking_the_well/hint8/text")

end
if  blockedWell_getNumSpelledNames() > 0 then
	disableLocationHint("hints/hint_group_unblocking_the_well/hint2/text")
end
if blockedWell_isNameSpelled("elfi") then
	disableLocationHint("hints/hint_group_unblocking_the_well/hint5/text")
end
if blockedWell_isNameSpelled("esther") then
	disableLocationHint("hints/hint_group_unblocking_the_well/hint3/text")
end
if blockedWell_isNameSpelled("alfonse") and 
	blockedWell_isNameSpelled("rainie") and 
	blockedWell_isNameSpelled("joseph") then
	disableLocationHint("hints/hint_group_unblocking_the_well/hint7/text")
end
if blockedWell_isNameSpelled("nathan") then
	disableLocationHint("hints/hint_group_unblocking_the_well/hint4/text")
end
if blockedWell_isNameSpelled("bessie") then
	disableLocationHint("hints/hint_group_unblocking_the_well/hint6/text")
end
if blockedWell_isNameSpelled("chance") and 
	blockedWell_isNameSpelled("elijah") and 
	blockedWell_isNameSpelled("ezra") then
	disableLocationHint("hints/hint_group_unblocking_the_well/hint8/text")
end


if isMinigameFinishedBlockedWell() then
	disableLocationHint("hints/hint_group_unblocking_the_well/hint3/text")
	disableLocationHint("hints/hint_group_unblocking_the_well/hint4/text")
	disableLocationHint("hints/hint_group_unblocking_the_well/hint5/text")
	disableLocationHint("hints/hint_group_unblocking_the_well/hint6/text")
	disableLocationHint("hints/hint_group_unblocking_the_well/hint7/text")
	disableLocationHint("hints/hint_group_unblocking_the_well/hint8/text")
	enableLocationHint("hints/hint_group_unblocking_the_well/hint9/text")
end
if isInState("french_quarter.street_2.hint_trigger_2", "triggered") then
	disableLocationHint("hints/hint_group_unblocking_the_well/hint9/text")
end
--Ordering the Apothecary Shelf	
if wasMinigameEnteredSortApothecaryItems() then
	enableLocationHint("hints/hint_group_ordering_shelves/hint1/text")
end
if (wasInventoryObjectFound("iodine") and
	wasInventoryObjectFound("morphine") and
	wasInventoryObjectFound("laudanum")) and not
	(hasInventoryObject("iodine") or 
	hasInventoryObject("morphine") or
	hasInventoryObject("laudanum")) then
	disableLocationHint("hints/hint_group_ordering_shelves/hint1/text")
	enableLocationHint("hints/hint_group_ordering_shelves/hint2/text")
end

if isMinigameFinishedWithoutReward_SortApothecaryItems() then
	disableLocationHint("hints/hint_group_ordering_shelves/hint1/text")
	disableLocationHint("hints/hint_group_ordering_shelves/hint2/text")
end

if isMinigameFinishedSortApothecaryItems() then
	disableLocationHint("hints/hint_group_ordering_shelves/hint1/text")
	disableLocationHint("hints/hint_group_ordering_shelves/hint2/text")
end
--Getting inside the Anton house
if isInState("french_quarter.street_1.to_morning_room", "active") then
	enableLocationHint("hints/hint_group_getting_inside_house/hint1/text")
end
if not isInState("french_quarter.street_2.well", "active") then
	disableLocationHint("hints/hint_group_getting_inside_house/hint1/text")	
end
if hasInventoryObject("antons_key") then
	enableLocationHint("hints/hint_group_getting_inside_house/hint2/text")
end
if isInState("french_quarter.street_1.to_morning_room", "opened") then
	disableLocationHint("hints/hint_group_getting_inside_house/hint1/text")
	disableLocationHint("hints/hint_group_getting_inside_house/hint2/text")
end
--Curing Mrs Anton's ghost
if not isInState("french_quarter.morning_room.bag", "preactive") then
	enableLocationHint("hints/hint_group_curing_the_ghost/hint1/text")
end
if isMinigameFinishedSortApothecaryItems() then
	disableLocationHint("hints/hint_group_curing_the_ghost/hint1/text")
	enableLocationHint("hints/hint_group_curing_the_ghost/hint2/text")
end
if isSpellEnabled("aconite_antidote") then
	disableLocationHint("hints/hint_group_curing_the_ghost/hint2/text")
	enableLocationHint("hints/hint_group_curing_the_ghost/hint3/text")
end
if wasInventoryObjectFound("antidote") then
	disableLocationHint("hints/hint_group_curing_the_ghost/hint3/text")
end
if hasInventoryObject("antidote") then
	enableLocationHint("hints/hint_group_curing_the_ghost/hint4/text")
end
if wasInventoryObjectFound("antidote") and not
	hasInventoryObject("antidote") then
	disableLocationHint("hints/hint_group_curing_the_ghost/hint4/text")
	enableLocationHint("hints/hint_group_curing_the_ghost/hint5/text")
end
if isInState("french_quarter.diary_inset.diary", "clicked") then
	disableLocationHint("hints/hint_group_curing_the_ghost/hint5/text")
end
	
	--Chapter 6 hints
--to do
if isInState("cemetery.gates.hint_trigger", "triggered") then
	enableLocationHint("hints/hint_group_to_voodoo_shop/hint1/text")
	enableLocationHint("hints/hint_group_to_voodoo_circle/hint1/text")
	enableLocationHint("hints/hint_group_to_voodoo_circle/hint2/text")
	enableLocationHint("hints/hint_group_to_voodoo_circle/hint3/text")
end
if isInState("cemetery.voodoo_shop.bird_fish_trigger", "triggered") then
	disableLocationHint("hints/hint_group_to_voodoo_shop/hint1/text")
	enableLocationHint("hints/hint_group_finding_the_curse/hint1/text")
	enableLocationHint("hints/hint_group_finding_the_curse/hint2/text")
end
if wasInventoryObjectFound("voodoo_key") then
	disableLocationHint("hints/hint_group_finding_the_curse/hint1/text")
end
if isMinigameFinishedSkullsObey() then
	disableLocationHint("hints/hint_group_finding_the_curse/hint2/text")
	enableLocationHint("hints/hint_group_finding_the_curse/hint3/text")
end
if isSpellEnabled("curse_of_fire") or 
	wasInventoryObjectFound("voodoo_doll") then
	disableLocationHint("hints/hint_group_finding_the_curse/hint3/text")
end
if wasInventoryObjectFound("cube_1") and
	wasInventoryObjectFound("cube_2") then
	disableLocationHint("hints/hint_group_to_voodoo_circle/hint1/text")
	enableLocationHint("hints/hint_group_to_voodoo_circle/hint4/text")
end
if wasInventoryObjectFound("cube_1") then
	disableLocationHint("hints/hint_group_to_voodoo_circle/hint2/text")
end
if wasInventoryObjectFound("cube_2") then
	disableLocationHint("hints/hint_group_to_voodoo_circle/hint3/text")
end
if (wasInventoryObjectFound("cube_1") and
	wasInventoryObjectFound("cube_2")) and not
	(hasInventoryObject("cube_1") or
	hasInventoryObject("cube_2")) then
	disableLocationHint("hints/hint_group_to_voodoo_circle/hint4/text")
end
if isInState("cemetery.gates.gates_mask", "assembled") then
	enableLocationHint("hints/hint_group_to_voodoo_circle/hint5/text")
end
if isMinigameFinishedCemeteryGate() then
	disableLocationHint("hints/hint_group_to_voodoo_circle/hint5/text")
end
--What to do at the Grave
if isInState("cemetery.gates.gates_mask", "opened") and 
	isSpellEnabled("curse_of_fire") then
	enableLocationHint("hints/hint_group_grave/hint1/text")
end
if voodooCircle_isCircleCleared() then
	disableLocationHint("hints/hint_group_grave/hint1/text")
	enableLocationHint("hints/hint_group_grave/hint2/text")
end
if voodooCircle_isCircleDrawn() then
	disableLocationHint("hints/hint_group_grave/hint2/text")
	enableLocationHint("hints/hint_group_grave/hint3/text")
	enableLocationHint("hints/hint_group_grave/hint4/text")
	enableLocationHint("hints/hint_group_grave/hint5/text")
end
if wasInventoryObjectFound("cage") and not
	hasInventoryObject("cage") then
	disableLocationHint("hints/hint_group_grave/hint3/text")
 end
if wasInventoryObjectFound("coconut") and not
	hasInventoryObject("coconut") then
	disableLocationHint("hints/hint_group_grave/hint4/text")
 end	
if wasInventoryObjectFound("rum") and not
	hasInventoryObject("rum") then
	disableLocationHint("hints/hint_group_grave/hint5/text")
 end
--voodoo doll
if isMinigameFinishedVoodooCircle() then
	enableLocationHint("hints/hint_group_making_doll/hint1/text")
end
if isSpellEnabled("voodoo_doll") then
	disableLocationHint("hints/hint_group_making_doll/hint1/text")
	enableLocationHint("hints/hint_group_making_doll/hint2/text")
	enableLocationHint("hints/hint_group_making_doll/hint3/text")
	enableLocationHint("hints/hint_group_making_doll/hint4/text")
	enableLocationHint("hints/hint_group_making_doll/hint5/text")
	enableLocationHint("hints/hint_group_making_doll/hint6/text")
	enableLocationHint("hints/hint_group_making_doll/hint7/text")
	enableLocationHint("hints/hint_group_making_doll/hint8/text")
	enableLocationHint("hints/hint_group_making_doll/hint9/text")
	enableLocationHint("hints/hint_group_making_doll/hint10/text")
end
if wasInventoryObjectFound("sticks") then
	disableLocationHint("hints/hint_group_making_doll/hint2/text")
end
if wasInventoryObjectFound("string") then
	disableLocationHint("hints/hint_group_making_doll/hint3/text")
end	
if wasInventoryObjectFound("moss") then
	disableLocationHint("hints/hint_group_making_doll/hint4/text")
end	
if wasInventoryObjectFound("muslin_bandage") then
	disableLocationHint("hints/hint_group_making_doll/hint5/text")
end	
if wasInventoryObjectFound("glue") then
	disableLocationHint("hints/hint_group_making_doll/hint6/text")
end	
if wasInventoryObjectFound("two_buttons") then
	disableLocationHint("hints/hint_group_making_doll/hint7/text")
end		
if wasInventoryObjectFound("lalauries_hair") then
	disableLocationHint("hints/hint_group_making_doll/hint8/text")
end		
if wasInventoryObjectFound("lipstick") then
	disableLocationHint("hints/hint_group_making_doll/hint9/text")
end		
if wasInventoryObjectFound("lalauries_dress_piece") then
	disableLocationHint("hints/hint_group_making_doll/hint10/text")
end		
if (wasInventoryObjectFound("sticks") and
	wasInventoryObjectFound("string") and
	wasInventoryObjectFound("moss") and
	wasInventoryObjectFound("muslin_bandage") and
	wasInventoryObjectFound("glue") and
	wasInventoryObjectFound("two_buttons") and
	wasInventoryObjectFound("lalauries_hair") and
	wasInventoryObjectFound("lipstick") and
	wasInventoryObjectFound("sewing_kit") and
	wasInventoryObjectFound("lalauries_dress_piece")) then
	enableLocationHint("hints/hint_group_making_doll/hint11/text")	
end
if voodooDoll_areIngredientsReady() then
	disableLocationHint("hints/hint_group_making_doll/hint11/text")
	enableLocationHint("hints/hint_group_making_doll/hint12/text")
end	
if isMinigameFinishedVoodooDoll() then
	disableLocationHint("hints/hint_group_making_doll/hint12/text")
end

if wasInventoryObjectFound("voodoo_doll") then
	enableLocationHint("hints/hint_group_making_doll/hint13/text")
end	
	
if wasInventoryObjectFound("voodoo_doll") and not
	hasInventoryObject("voodoo_doll") then
	disableLocationHint("hints/hint_group_making_doll/hint13/text")
 end	
	
	
	
	
	
	
	
	
	
	
	
