extends Label

func _process(_delta: float)->void:
	var p:Vector3 = gameFrontend.GetPlayerPosition();
	text = "Position: %.2f %.2f %.2f       %s" % [p.x, p.y, p.z, ("ON GROUND" if gameFrontend.GetIsPlayerOnGround() else "IN AIR")];
