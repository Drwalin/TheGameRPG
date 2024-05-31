extends Control;

var username: String = "";
var password: String = "";

func _on_connect_to_server_pressed():
	var address:String = $Address.text;
	var port:int = int($PortNumber.text);
	gameFrontend.Connect(address, port);
	
func _on_exit_pressed():
	get_tree().quit();
	
func _on_disconnect_pressed():
	gameFrontend.Disconnect();

func _OnLoginPressed():
	username = $Username.text;
	password = $Password.text;
	if username != "":
		gameFrontend.Login(username, password);
		hide();
	else:
		print("_OnLoginPressed");

func _input(event)->void:
	if event is InputEventKey:
		if event.keycode == KEY_ESCAPE && event.pressed:
			if visible:
				hide();
			else:
				show();

func _ready():
	_on_connect_to_server_pressed();

func _process(delta):
	pass;
