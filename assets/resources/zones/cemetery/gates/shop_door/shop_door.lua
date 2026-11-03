---missing_bell
onStateEnter = function (this)
	setVisible(this, true)
end

---assembled
onStateEnter = function (this)
	setTexture(this, "shop_bell")
end

---open
onStateEnter = function (this)
	setTexture(this, "shop_open")
end