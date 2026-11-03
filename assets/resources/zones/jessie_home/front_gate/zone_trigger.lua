---start

---c2end
onStateEnter = function (this)
	allowZoneAccess("lalaurie")
end
---c3mid
onStateEnter = function (this)
	allowZoneAccess("jessie_home")
end
---c3end
onStateEnter = function (this)
	allowZoneAccess("cajun")
	disallowZoneAccess("lalaurie")
	disallowZoneAccess("jessie_home")
end
---c4mid
onStateEnter = function (this)
	allowZoneAccess("jessie_home")
end
---c4end
onStateEnter = function (this)
	allowZoneAccess("french_quarter")
	disallowZoneAccess("jessie_home")
	disallowZoneAccess("cajun")
end
---c5mid
onStateEnter = function (this)
	allowZoneAccess("jessie_home")
end
---c5end
onStateEnter = function (this)
	allowZoneAccess("cemetery")
	disallowZoneAccess("jessie_home")
	disallowZoneAccess("french_quarter")
	removeInventoryObject("binoculars")
end
---c6mid1
--start
onStateEnter = function (this)
	allowZoneAccess("cajun")
	allowZoneAccess("cemetery")
end
---c6mid2
--doll_turned_apart
onStateEnter = function (this)
	allowZoneAccess("jessie_home")
	allowZoneAccess("lalaurie")
	allowZoneAccess("french_quarter")
end
---c6mid3
--lalaurie_clear
onStateEnter = function (this)
	disallowZoneAccess("lalaurie")
end
---c6mid4
--cajun_clear
onStateEnter = function (this)
	disallowZoneAccess("cajun")
end
---c6mid5
--fq_clear
onStateEnter = function (this)
	disallowZoneAccess("french_quarter")
end