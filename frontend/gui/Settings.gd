extends Control

func _ready():
	SetMouseSensitivity(25);

func _on_main_menu_pressed():
	get_parent().SwitchToMenu($"../MainMenu");

func SetMouseSensitivity(value: float):
	value = clamp(value, 1, 100);
	$VBoxContainer/TabContainer/Sterowanie/MouseSensitivity/Slider.value = value;
	$VBoxContainer/TabContainer/Sterowanie/MouseSensitivity/Numerical.text = "%d" % int(value);
	gameFrontend.SENSITIVITY = value/9000.0;
	

func _on_mouse_sensitivity_numerical_changed(value: String):
	SetMouseSensitivity(int(value));

func _on_mouse_sensitivity_slider_changed(value: float):
	SetMouseSensitivity(value);
