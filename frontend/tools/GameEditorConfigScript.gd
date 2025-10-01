@tool
extends GameEditorConfig

@export_tool_button("Save Map File", "File") var save_map_action:Callable = save_map_action_func
func save_map_action_func():
	print(gameFrontend);
	gameFrontend.InternalReady();
	SaveSceneInt(gameFrontend.GetThisAsInt());
