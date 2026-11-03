---state01
sm01objectsAdded = false
sm02objectsAdded = false
sm03objectsAdded = false
sm04objectsAdded = false
sm05objectsAdded = false
pz01objectsAdded = false
pz05objectsAdded = false
pz07objectsAdded = false
pz08objectsAdded = false
pz09objectsAdded = false
pz11objectsAdded = false
pz12objectsAdded = false
pz14objectsAdded = false
revealSpiritsObjectAdded = false

onLMouseUp = function (this)

--	gotoMinigameBanishSpirits()
--	gotoMinigameGraveyardGrease()
--	gotoMinigameSpiritRevealingMixture()
--	gotoMinigameAconiteAntidote()
--	gotoMinigameVoodooDoll()

--	gotoMinigameWindowCleaning()
--	gotoMinigameHexBones()
--	gotoMinigameOuijiBoard()
--	gotoMinigamePiano()
--	gotoMinigameDoveCote()
--	gotoMinigameRats()
--	gotoMinigameCellarDoors()
--	gotoMinigameSortApothecaryItems()
--	gotoMinigameSpiritLock()
--	gotoMinigameBlockedWell()
--	gotoMinigameSail()
--	gotoMinigameLeeches()
--	gotoMinigameCemeteryGate()
--	gotoMinigameVoodooCircle()
--	gotoMinigameSkullsObey()

--[[
	if not sm01objectsAdded then
		addInventoryObject("salt")
		addInventoryObject("clove")
		addInventoryObject("mint")
		addInventoryObject("solomon_seal")
		addInventoryObject("devils_shoestring")
		addInventoryObject("envenomed_apple")
		addInventoryObject("bottle")
		addInventoryObject("red_string")
		sm01objectsAdded = true
	end
	gotoMinigameBanishSpirits()
]]
--[[
	if not sm02objectsAdded then
		addInventoryObject("lemon")
		addInventoryObject("tears")
		addInventoryObject("onion_flower")
		addInventoryObject("garlic_head")
		addInventoryObject("white_pepper")
		addInventoryObject("jug_of_water")
		addInventoryObject("bowl")
		sm02objectsAdded = true
	end
	gotoMinigameGraveyardGrease()
]]
--[[
	if not sm03objectsAdded then
		addInventoryObject("lily")
		addInventoryObject("white_paint")
		addInventoryObject("dirt")
		addInventoryObject("sulfur")
		addInventoryObject("feather")
		addInventoryObject("vial_with_blood")
		sm03objectsAdded = true
	end
	gotoMinigameSpiritRevealingMixture()
]]
--[[
	if not sm04objectsAdded then
		addInventoryObject("aspirin")
		addInventoryObject("baking_soda")
		addInventoryObject("yellow_monkshood")
		addInventoryObject("aconite")
		sm04objectsAdded = true
	end
	gotoMinigameAconiteAntidote()
]]
--[[
	if not sm05objectsAdded then
		addInventoryObject("sticks")
		addInventoryObject("string")
		addInventoryObject("moss")
		addInventoryObject("muslin_bandage")
		addInventoryObject("sewing_kit")
		addInventoryObject("glue")
		addInventoryObject("two_buttons")
		addInventoryObject("lipstick")
		addInventoryObject("lalauries_hair")
		addInventoryObject("lalauries_dress_piece")
		sm05objectsAdded = true
	end
	gotoMinigameVoodooDoll()
]]
--[[
	if not pz01objectsAdded then
		addInventoryObject("mixture_grease_cleaning")
		addInventoryObject("sponge")
		pz01objectsAdded = true
	end
	gotoMinigameWindowCleaning()
]]
--	gotoMinigameHexBones()
--	gotoMinigameOuijiBoard()
--	gotoMinigamePiano()
--[[
	if not pz05objectsAdded then
		addInventoryObject("rope")
		addInventoryObject("seeds")
		pz05objectsAdded = true
	end
	gotoMinigameDoveCote()
]]
--	gotoMinigameRats()
--[[
	if not pz07objectsAdded then
		addInventoryObject("ash")
		pz07objectsAdded = true
	end
	gotoMinigameCellarDoors()
]]
--[[
	if not pz08objectsAdded then
		addInventoryObject("iodine")
		addInventoryObject("laudanum")
		addInventoryObject("morphine")
		-- some wrong items for reactive text testing
		addInventoryObject("baking_soda")
		addInventoryObject("aspirin")
		addInventoryObject("apple")
		pz08objectsAdded = true
	end
	gotoMinigameSortApothecaryItems()
]]
--[[
	if not pz09objectsAdded then
		addInventoryObject("aemaet_piece")
		addInventoryObject("barbathioath_piece")
		addInventoryObject("ephphata_piece")
		addInventoryObject("harrahya_piece")
		addInventoryObject("kedavra_piece")
		addInventoryObject("matba_piece")
		addInventoryObject("selah_piece")
		addInventoryObject("yaldabaoth_piece")
		pz09objectsAdded = true
	end
	gotoMinigameSpiritLock()
]]
--[[
	blockedWell_addKnownName("Nathan")
	blockedWell_addKnownName("Chance")
	blockedWell_addKnownName("Ezra")
	blockedWell_addKnownName("Elijah")
	blockedWell_addKnownName("Bessie")
	blockedWell_addKnownName("Alfonse")
	blockedWell_addKnownName("Elfi")
	blockedWell_addKnownName("Joseph")
	blockedWell_addKnownName("Esther")
	blockedWell_addKnownName("Rainie")
	gotoMinigameBlockedWell()
]]
--[[
	if not pz11ObjectsAdded then
		addInventoryObject("sewing_kit")
		addInventoryObject("nightdress")
		addInventoryObject("shirt")
		addInventoryObject("bloomers")
		addInventoryObject("trowel")
		addInventoryObject("tablecloth")
		pz11ObjectsAdded = true
	end
	gotoMinigameSail()
]]
--[[
	if not pz12ObjectsAdded then
		addInventoryObject("orange_1")
		addInventoryObject("orange_2")
		addInventoryObject("orange_3")
		pz12ObjectsAdded = true
	end
	gotoMinigameLeeches()
]]
--	gotoMinigameCemeteryGate()

	if not pz14ObjectsAdded then
		addInventoryObject("chalk")
		addInventoryObject("rum")
		pz14ObjectsAdded = true
	end
	gotoMinigameVoodooCircle()

--	gotoMinigameSkullsObey()
--[[
	allowZoneAccess("lalaurie")
	gotoState("jessie_home.front_gate.front_gate_carriage", "active")
	addInventoryObject("spirit_revealing_mixture")
	addInventoryObject("brush")
	addAnywhereAppliable("lalaurie.back_yard", "spirit_revealing_mixture", "/zones/lalaurie/back_yard/reveal_spell/on_apply_mixture")
	addAnywhereAppliable("lalaurie.cellar", "spirit_revealing_mixture", "/zones/lalaurie/cellar/reveal_spell/on_apply_mixture")
	addAnywhereAppliable("lalaurie.gazebo_exterior", "spirit_revealing_mixture", "/zones/lalaurie/gazebo_exterior/reveal_spell/on_apply_mixture")
	addAnywhereAppliable("lalaurie.pond", "spirit_revealing_mixture", "/zones/lalaurie/pond/reveal_spell/on_apply_mixture")
	showTutorialHint("new_areas")
]]
end

onRMouseDown = function (this)

	if not isSpellBookEnabled() then
		enableSpellBook()
		enableSpell("graveyard_grease")
		enableSpell("banish_spirits")
		enableSpell("spirit_revealing_mixture")
		enableSpell("aconite_antidote")
		enableSpell("curse_of_fire")
		enableSpell("voodoo_doll")
	elseif not isPlantBookEnabled() then
		enablePlantBook()
		enablePlantBookPage("solomon_seal")
		enablePlantBookPage("devils_shoestring")
		enablePlantBookPage("onion_flower")
		enablePlantBookPage("lily_of_the_valley")
		enablePlantBookPage("orchid")
		enablePlantBookPage("yellow_monkshood")
	else
		openSpellBook("spirit_revealing_mixture", 3.0)
	end
end
