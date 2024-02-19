extends TestClassGaming


# Called when the node enters the scene tree for the first time.
func _ready():
	if Engine.is_editor_hint():
		super.set_process_mode(Node.PROCESS_MODE_DISABLED);
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	print("xD")
	pass
