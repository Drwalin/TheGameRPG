extends Label

func _process(_delta: float)->void:
	text = "Tick: %d\nPing: %d ms" % [gameFrontend.GetCurrentTick(), gameFrontend.GetPing()];
