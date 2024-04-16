extends Control

var username: String = "";
var password: String = "";

func _on_connect_to_server_pressed():
	var address:String = $Address.text;
	var port:int = int($PortNumber.text);
	gameFrontend.Connect(address, port);

func _OnLoginPressed():
	username = $Username.text;
	password = $Password.text;
	if username != "":
		gameFrontend.Login(username, password);
		hide();
	else:
		print("_OnLoginPressed");

# Called when the node enters the scene tree for the first time.
func _ready():
	_on_connect_to_server_pressed();


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if gameFrontend.IsConnecting():
		if gameFrontend.IsConnected():
			$ConnectionState.text = "Connected to server";
			$ConnectionState.set("theme_override_colors/font_color", Color(0.85, 0.85, 0.85));
		else:
			$ConnectionState.text = "Connecting to server...";
			$ConnectionState.set("theme_override_colors/font_color", Color(0.80, 0.80, 0.15));
	else:
		$ConnectionState.text = "Server disconnected!";
		$ConnectionState.set("theme_override_colors/font_color", Color(0.75, 0.15, 0.15));
