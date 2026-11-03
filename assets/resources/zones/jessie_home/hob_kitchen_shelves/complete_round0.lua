addInventoryObject("empty_jug")
gotoState("jessie_home.kitchen.kitchen_shelves", "inactive")
playSound("/sound/sfx/sfx_item_found")

if wasInventoryObjectFound("oil") then
	gotoState("jessie_home.kitchen.kitchen_shelves", "active_1")
end