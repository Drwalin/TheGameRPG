extends Label;

func _ready():
	pass;

func _process(delta):
	if gameFrontend.IsDisconnected():
		if !visible:
			show();
		text = "Server disconnected!";
		set("theme_override_colors/font_color", Color(0.75, 0.15, 0.15));
	elif gameFrontend.IsConnected():
		text = "Connected to server";
		set("theme_override_colors/font_color", Color(0.85, 0.85, 0.85));
		var v:bool = get_parent().find_child("MainMenu", false).visible;
		if visible != v:
			visible = v;
	else:
		if !visible:
			show();
		text = "Connecting to server...";
		set("theme_override_colors/font_color", Color(0.80, 0.80, 0.15));
