shader_type spatial;

render_mode unshaded;
render_mode cull_disabled;
render_mode shadows_disabled;
render_mode ambient_light_disabled;
render_mode fog_disabled;
render_mode blend_add;

void vertex() {
}

void fragment() {
	float dist = 1.0 - clamp(length(UV*2.0 - vec2(1.0, 1.0)), 0.0, 1.0);
	ALBEDO = vec3(0.2, 0.01, 0.35)*2.5 * ALPHA;
	ALPHA = COLOR.a * dist*dist;
}
