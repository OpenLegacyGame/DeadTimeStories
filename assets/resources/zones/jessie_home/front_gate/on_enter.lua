if isInState("jessie_home.front_gate.beta_trigger", "end_beta") then
	scriptOnClose =
	'endGame()'
	showMessageDialog("%Congratulations!\nYou have completed all the adventures available in Beta Test edition of Deadtime Stories.\n\nPlease come back and join Jessie and Edward in the Spirit World of New Orlean once the game is out in April.%", scriptOnClose)
end

if (isInState("jessie_home.front_gate.front_gate_carriage", "active")) then
	showTutorialHint("carriage")
end