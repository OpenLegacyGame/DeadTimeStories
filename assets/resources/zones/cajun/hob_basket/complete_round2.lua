if hasInventoryObject("doll_head") then
	removeInventoryObject("doll_head")
	addInventoryObject("baby_doll")
	showMessageDialog("%你找到了娃娃丢失的头. 现在娃娃完成了.%")
	else
	addInventoryObject("doll_without_head")
end

gotoState("cajun.mangrove_circle.basket", "inactive")
playSound("/sound/sfx/sfx_item_found")