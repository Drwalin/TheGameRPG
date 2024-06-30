extends VBoxContainer

func AddMonitor(text: String, unit: String, _monitor: int, format:String = "%f", multiplier:float = 1.0)->void:
	var mon:SinglePerformanceMonitor = preload("res://scripts/performance_monitor/SinglePerformanceMonitor.tscn").instantiate();
	mon.SetMonitor(text, unit, _monitor, format, multiplier);
	add_child(mon);
	mon.owner = self;

func _ready():
	AddMonitor("TIME_FPS", "", Performance.Monitor.TIME_FPS, "%.0f");
	AddMonitor("TIME_PROCESS", "ms", Performance.Monitor.TIME_PROCESS, "%.3f", 1000.0);
	AddMonitor("TIME_PHYSICS_PROCESS", "ms", Performance.Monitor.TIME_PHYSICS_PROCESS, "%.3f", 1000.0);
	AddMonitor("MEMORY_STATIC", "MiB", Performance.Monitor.MEMORY_STATIC, "%.2f", 1.0/(1024.0*1024.0));
	AddMonitor("OBJECT_COUNT", "", Performance.Monitor.OBJECT_COUNT, "%.0f");
	AddMonitor("OBJECT_RESOURCE_COUNT", "", Performance.Monitor.OBJECT_RESOURCE_COUNT, "%.0f");
	AddMonitor("OBJECT_NODE_COUNT", "", Performance.Monitor.OBJECT_NODE_COUNT, "%.0f");
	AddMonitor("OBJECT_ORPHAN_NODE_COUNT", "", Performance.Monitor.OBJECT_ORPHAN_NODE_COUNT, "%.0f");
	AddMonitor("RENDER_TOTAL_OBJECTS_IN_FRAME", "", Performance.Monitor.RENDER_TOTAL_OBJECTS_IN_FRAME, "%.0f");
	AddMonitor("RENDER_TOTAL_PRIMITIVES_IN_FRAME", "verts", Performance.Monitor.RENDER_TOTAL_PRIMITIVES_IN_FRAME, "%.0f");
	AddMonitor("RENDER_TOTAL_DRAW_CALLS_IN_FRAME", "", Performance.Monitor.RENDER_TOTAL_DRAW_CALLS_IN_FRAME, "%.0f");
	AddMonitor("RENDER_VIDEO_MEM_USED", "MiB", Performance.Monitor.RENDER_VIDEO_MEM_USED, "%.2f", 1.0/(1024.0*1024.0));
	AddMonitor("RENDER_TEXTURE_MEM_USED", "MiB", Performance.Monitor.RENDER_TEXTURE_MEM_USED, "%.2f", 1.0/(1024.0*1024.0));
	AddMonitor("RENDER_BUFFER_MEM_USED", "MiB", Performance.Monitor.RENDER_BUFFER_MEM_USED, "%.2f", 1.0/(1024.0*1024.0));
