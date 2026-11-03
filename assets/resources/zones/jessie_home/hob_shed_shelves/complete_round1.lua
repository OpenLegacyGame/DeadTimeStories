addInventoryObject("oil")
gotoState("jessie_home.shed.to_HOB_shed_shelves", "inactive")
playSound("/sound/sfx/sfx_item_found")

if wasInventoryObjectFound("empty_jug") then
	gotoState("jessie_home.kitchen.kitchen_shelves", "active_1")
end