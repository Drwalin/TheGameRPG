extends Control

var userName: String = "";
var realmName: String = "";

func _on_connect_to_server_pressed():
	var address:String = $Address.text;
	var port:int = int($PortNumber.text);
	connectionClient.Connect(address, port);

func UpdateRealmsListUI()->void:
	var realms = connectionClient.GetRealms();
	$RealmList.clear();
	for realm in realms:
		$RealmList.add_item(realm);
	if realms.size() > 0:
		$RealmList.deselect_all();
		$RealmList.select(0);
		_OnSelectedRealm(0);

func _OnSelectedRealm(index: int):
	realmName = $RealmList.get_item_text(index);
	print("Selected realm: ", realmName);

func _OnLoginPressed():
	userName = $Username.text;
	if userName != "" && realmName != "":
		connectionClient.SetUsername(userName);
		connectionClient.EnterRealm(realmName);
		print("Entering realm: '", realmName, "', as '", userName, "'");
		hide();
	else:
		print("_OnLoginPressed");

# Called when the node enters the scene tree for the first time.
func _ready():
	connectionClient.connect("_ReceivedRealmsList", UpdateRealmsListUI);


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if connectionClient.client != null:
		if connectionClient.client.IsReadyToUse():
			$ConnectionState.text = "Connected to server";
			$ConnectionState.set("theme_override_colors/font_color", Color(0.85, 0.85, 0.85));
		else:
			$ConnectionState.text = "Connecting to server...";
			$ConnectionState.set("theme_override_colors/font_color", Color(0.80, 0.80, 0.15));
	else:
		$ConnectionState.text = "Server disconnected!";
		$ConnectionState.set("theme_override_colors/font_color", Color(0.75, 0.15, 0.15));
