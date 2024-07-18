extends Control
class_name SettingsConfigurationClass;

static var save:bool = false;
static var toSave:float = 0;

func _ready():
	SetMouseSensitivity(ProjectSettings.get_setting("game_settings/mouse/sensitivity", 25));
	_on_enable_monitoring_toggled(ProjectSettings.get_setting("game_settings/monitoring/monitor", false));
	_on_show_player_position_toggled(ProjectSettings.get_setting("game_settings/monitoring/player_position", false));
	_on_show_ping_toggled(ProjectSettings.get_setting("game_settings/monitoring/ping", false));
	_on_enable_point_shadows_toggled(ProjectSettings.get_setting("game_settings/graphics/point_shadows", true));
	ProjectSettings.set_initial_value("rendering/renderer/rendering_method", null);
	
	match ProjectSettings.get_setting("rendering/renderer/rendering_method"):
		"forward_plus":
			_on_rendering_driver_value_changed(2);
		"mobile":
			_on_rendering_driver_value_changed(1);
		"gl_compatibility":
			_on_rendering_driver_value_changed(0);

func _process(dt:float):
	toSave -= dt;
	if save && (toSave <= 0.0 || visible==false):
		SettingsConfigurationClass.Save();

static func Save():
	if save:
		ProjectSettings.save_custom("override.cfg");
		save = false;
		toSave = ProjectSettings.get_setting("game_settings/settings/file_saving/delay", 10.0);

func _exit_tree():
	SettingsConfigurationClass.Save();

func _notification(what):
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		SettingsConfigurationClass.Save();
		get_tree().quit();

func _on_main_menu_pressed():
	get_parent().SwitchToMenu($"../MainMenu");

func SetMouseSensitivity(value: float):
	value = clamp(value, 1, 100);
	$VBoxContainer/TabContainer/Sterowanie/MouseSensitivity/Slider.value = value;
	$VBoxContainer/TabContainer/Sterowanie/MouseSensitivity/Numerical.text = "%d" % int(value);
	ProjectSettings.set_setting("game_settings/mouse/sensitivity", value);
	gameFrontend.SENSITIVITY = value/9000.0;
	save = true;

func _on_mouse_sensitivity_numerical_changed(value: String):
	SetMouseSensitivity(int(value));

func _on_mouse_sensitivity_slider_changed(value: float):
	SetMouseSensitivity(value);

func _on_enable_monitoring_toggled(toggled_on: bool):
	$"../Hud/PerformanceStatistics".visible = toggled_on;
	save = true;
	ProjectSettings.set_setting("game_settings/monitoring/monitor", toggled_on);
	$VBoxContainer/TabContainer/Monitoring/HBoxContainer/EnableMonitoring.button_pressed = toggled_on;

func _on_show_player_position_toggled(toggled_on: bool):
	$"../Hud/Position".visible = toggled_on;
	save = true;
	ProjectSettings.set_setting("game_settings/monitoring/player_position", toggled_on);
	$VBoxContainer/TabContainer/Monitoring/HBoxContainer2/ShowPlayerPosition.button_pressed = toggled_on;

func _on_show_ping_toggled(toggled_on: bool):
	$"../Hud/Ping".visible = toggled_on;
	save = true;
	ProjectSettings.set_setting("game_settings/monitoring/ping", toggled_on);
	$VBoxContainer/TabContainer/Monitoring/HBoxContainer3/ShowPing.button_pressed = toggled_on;

func _on_enable_point_shadows_toggled(toggled_on):
	$VBoxContainer/TabContainer/Graphics/HBoxContainer4/EnablePointShadows.button_pressed = toggled_on;
	save = true;
	ProjectSettings.set_setting("game_settings/graphics/point_shadows", toggled_on);

func _on_rendering_driver_value_changed(value):
	$VBoxContainer/TabContainer/Graphics/RendererDriver/Slider.value = value;
	value = clamp(value, 0, 2);
	value = int(value);
	match value:
		2:
			ProjectSettings.set_setting("rendering/renderer/rendering_method", "forward_plus");
		1:
			ProjectSettings.set_setting("rendering/renderer/rendering_method", "mobile");
		_:
			ProjectSettings.set_setting("rendering/renderer/rendering_method", "gl_compatibility");
	save = true;
