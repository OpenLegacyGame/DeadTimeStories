dialogCloseScript = "gotoState(\"jessie_home.parlor.ray_dust\", \"shine\") gotoLocation(\"jessie_home.parlor\")"
showMessageDialog( getString("puzzles/window_cleaning/congratulations"), dialogCloseScript)
gotoState("jessie_home.parlor.window", "inactive")
if hasInventoryObject("sponge") then
	removeInventoryObject("sponge")
end
if hasInventoryObject("sponge_wet") then
	removeInventoryObject("sponge_wet")
end