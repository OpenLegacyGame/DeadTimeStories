scriptOnClose =
	'gotoState("lalaurie.back_yard.dove_cote", "inactive")'..
	'addInventoryObject("feather")'..
	'gotoState("lalaurie.pond.fishing_basket", "active_1")'..
	'gotoLocation("lalaurie.back_yard")'
showMessageDialogWithImage( getString("puzzles/dove_cote/congratulations"), "/inventory/feather", scriptOnClose )