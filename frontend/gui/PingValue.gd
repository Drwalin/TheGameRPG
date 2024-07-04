extends Label

func _process(delta):
	text = "Tick: %d\nPing: %d ms" % [gameFrontend.GetCurrentTick(), gameFrontend.GetPing()];
