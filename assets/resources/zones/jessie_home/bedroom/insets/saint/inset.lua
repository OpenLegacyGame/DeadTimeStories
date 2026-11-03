---active
onStateEnter = function (this)
	setInteractive (this, true)	
end

---cry
onStateEnter = function (this)
	setDefaultCursor (this)
	setInteractive (this, true)
end

---inactive
onStateEnter = function (this)
	setDefaultCursor (this)
end
