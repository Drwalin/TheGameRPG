extends Control;

var username: String = "";
var password: String = "";

func _on_connect_to_server_pressed():
	var address:String = $Address.text;
	var port:int = int($PortNumber.text);
	gameFrontend.Connect(address, port);
	SaveAddressAndPort();

func SaveAddressAndPort():
	ProjectSettings.set_setting("game_settings/server_addres/address", $Address.text);
	ProjectSettings.set_setting("game_settings/server_addres/port", $PortNumber.text);
	SettingsConfigurationClass.save = true;
	SettingsConfigurationClass.Save();
	

func _on_exit_pressed():
	gameFrontend.Disconnect();
	get_tree().quit();
	
func _on_disconnect_pressed():
	gameFrontend.Disconnect();

func _OnLoginPressed():
	username = $Username.text;
	password = $Password.text;
	if username != "":
		gameFrontend.Login(username, password);

func _input(event)->void:
	if event is InputEventKey:
		if event.keycode == KEY_ESCAPE && event.pressed:
			if visible:
				get_parent().SwitchToMenu($"../Hud");
			else:
				get_parent().SwitchToMenu(self);

func _ready():
	$Address.text = ProjectSettings.get_setting("game_settings/server_addres/address", "127.0.0.1");
	$PortNumber.text = ProjectSettings.get_setting("game_settings/server_addres/port", "25369");
	_on_connect_to_server_pressed();

func _on_settings_pressed():
	get_parent().SwitchToMenu($"../Settings");

func _on_help_pressed():
	get_parent().SwitchToMenu($"../Help");
