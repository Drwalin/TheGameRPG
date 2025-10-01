@tool
extends GameEditorConfig

@export_tool_button("Save Map File", "File") var save_map_action:Callable = save_map_action_func
func save_map_action_func():
	gameFrontend.InternalReady();
	SaveSceneInt(gameFrontend.GetThisAsInt());

var count : int = 0;

func _process(_delta: float) -> void:
	if (!Engine.is_editor_hint()):
		count = count + 1;
		if (count == 4):
			gameFrontend.InternalReady();
			SaveSceneInt(gameFrontend.GetThisAsInt());
			self.get_tree().quit();
