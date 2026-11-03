gotoState("cemetery.gates.gates_mask", "opened")
scriptAfterDialog =
	"if dialogResult == 'ok' then playSound('/sound/sfx/mg/cemetery_gate/mg_open_gates') end\n"..
	"exitMinigame()"
showMessageDialog( getString("puzzles/cemetery_gate/congratulations"), scriptAfterDialog )
