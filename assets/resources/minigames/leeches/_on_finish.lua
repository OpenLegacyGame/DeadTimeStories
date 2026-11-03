gotoState("cajun.swamp_island.wringer_minigame", "inactive")
gotoState("cajun.swamp_island.wringer_mask", "inactive")


showMessageDialogWithImage(
	getString("puzzles/leeches/congratulations"),
	"/inventory/tablecloth",
	'exitMinigame() addInventoryObject("tablecloth")'
	)