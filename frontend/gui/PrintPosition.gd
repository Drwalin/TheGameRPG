extends Label

func _process(_delta: float)->void:
	var p:Vector3 = gameFrontend.GetPlayerPosition();
	text = "Position: %.2f %.2f %.2f" % [p.x, p.y, p.z];
