extends Label

func _process(delta):
	var p:Vector3 = gameFrontend.GetPlayerPosition();
	text = "Position: %.2f %.2f %.2f" % [p.x, p.y, p.z];
