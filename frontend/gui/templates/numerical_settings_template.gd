extends HBoxContainer

@export var defaultValue:int;
@export var min:int:
	set(value):
		min = value;
		if $Slider:
			$Slider.min_value = float(value);
@export var max:int:
	set(value):
		max = value;
		if $Slider:
			$Slider.max_value = float(value);
@export var varName:String:
	set(value):
		varName = value;
		if $Label:
			$Label.text = value;
@export var settingPath:String;

signal on_value_changed(value:int);

func _ready() -> void:
	$Label.text = varName;
	$Slider.min_value = min;
	$Slider.max_value = max;
	SetValue(ProjectSettings.get_setting(settingPath, defaultValue));

func SetValue(value: float)->void:
	value = clamp(value, min, max);
	$Slider.value = value;
	$Numerical.text = "%d" % int(value);
	ProjectSettings.set_setting(settingPath, value);
	on_value_changed.emit(value);
	SettingsConfigurationClass.save = true;

func _on_numerical_text_submitted(value: String)->void:
	SetValue(int(value));

func _on_slider_value_changed(value: float) -> void:
	SetValue(value);
