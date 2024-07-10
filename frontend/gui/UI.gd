extends Node

@onready var currentMenu:Control;

func _ready():
	SwitchToMenu($"MainMenu");

func SwitchToMenu(menu):
	if menu is String:
		for child in get_children():
			if child is Control:
				if menu == child.name:
					currentMenu = child;
					SwitchToMenu(child);
					return;
		print("Menu: `", menu, "` is not valid");
	elif menu is Control:
		var found:bool = false;
		for child in get_children():
			if child is Control:
				if menu == child:
					found = true;
		if found:
			for child in get_children():
				if child is Control:
					child.visible = (menu == child);
		else:
			print("Menu: `", menu, "` is not valid");
		
