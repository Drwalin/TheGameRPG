class_name SinglePerformanceMonitor;
extends HBoxContainer;

var monitor:int;
var format:String;
var multiplier:float;

func SetMonitor(text: String, unit: String, _monitor: int, _format:String = "%f", _multiplier:float = 1.0)->void:
	monitor = _monitor;
	$Label.text = text;
	$Unit.text = unit;
	format = _format;
	multiplier = _multiplier;

func _process(_dt: float)->void:
	$Value.text = format % (Performance.get_monitor(monitor) * multiplier);
