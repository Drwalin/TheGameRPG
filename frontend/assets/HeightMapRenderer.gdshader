shader_type spatial;

render_mode skip_vertex_transform;
render_mode cull_disabled;

uniform sampler2D terrainData : filter_nearest;//, repeat_disable, hint_default_black;
uniform ivec2 size;
uniform sampler2D materialColorsRamp;
uniform vec3 scaleSize;

float GetHeight(ivec2 pos)
{
	if (pos.x < 0 || pos.y < 0 || pos.x > size.x || pos.y > size.y)
		return 0.0;
	vec4 c = texture(terrainData, (vec2(pos)) / vec2(size+1) + 0.5/vec2(size));
	return (c.r + c.g / 256.0) * 64.0;
}

int GetMaterial(ivec2 pos)
{
	if (pos.x < 0 || pos.y < 0 || pos.x > size.x || pos.y > size.y)
		return 0;
	vec4 c = texture(terrainData, (vec2(pos)) / vec2(size+1) + 0.5/vec2(size));
	return int(c.b * 255.0);
}

vec3 hsv2rgb(vec3 c) {
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 GetMaterialColor(int material, vec3 pos)
{
	return hsv2rgb(vec3(float(material) / 255.0, 1, 1));
}

vec3 GetPos(int id)
{
	int triId = id / 3;
	int vertId = id % 3;
	int quadId = triId / 2;
	int triVer = triId % 2;

	int vsId = id % 6;

	int Y = quadId / (size.x);
	int X = quadId % (size.x);

	const vec2 vs[6] = {
			vec2(0,0),
			vec2(0,1),
			vec2(1,1),
			vec2(0,0),
			vec2(1,1),
			vec2(1,0)
	};

	vec2 p;
	p.x = float(X);
	p.y = float(Y);
	p += vs[vsId];

	vec3 pos;
	pos.x = p.x;
	pos.z = p.y;
	pos.y = GetHeight(ivec2(p));

	return pos;
}

varying float height;
varying vec4 materials;
varying vec2 materialOffset;

void vertex() {
	int id = VERTEX_ID;
	int vid = id % 3;
	int idBase = id - vid;

	vec3 vecs[3];
	for (int i=0; i<3; ++i)
		vecs[i] = GetPos(idBase+i);

	vec3 normal = normalize(cross((vecs[2]-vecs[0])*scaleSize, (vecs[1]-vecs[0])*scaleSize));
	vec3 tangent = normalize(cross(normal, vec3(1,0,0)));
	vec3 binoraml = normalize(cross(tangent, normal));

	materials[0] = float(GetMaterial(ivec2(GetPos(idBase).xz)));
	materials[1] = float(GetMaterial(ivec2(GetPos(idBase).xz) + ivec2(1, 0)));
	materials[2] = float(GetMaterial(ivec2(GetPos(idBase).xz) + ivec2(0, 1)));
	materials[3] = float(GetMaterial(ivec2(GetPos(idBase).xz) + ivec2(1, 1)));

	height = vecs[vid].y;
	materialOffset = vecs[vid].xz - vecs[0].xz;

	normal = vec3(0,-1,0);

    VERTEX = (MODELVIEW_MATRIX * vec4(vecs[vid] * scaleSize, 1.0)).xyz;
    NORMAL = normalize((MODELVIEW_MATRIX * vec4(normal, 0.0)).xyz);
    BINORMAL = normalize((MODELVIEW_MATRIX * vec4(binoraml, 0.0)).xyz);
    TANGENT = normalize((MODELVIEW_MATRIX * vec4(tangent, 0.0)).xyz);
	UV = vecs[vid].xz / vec2(size);


	if (idBase < 0) {
		VERTEX = (MODELVIEW_MATRIX * vec4(0,0,0, 1.0)).xyz;
	}
}

void fragment() {
	vec3 m[4];
	m[0] = GetMaterialColor(int(materials[0]), vec3(UV.x, height, UV.y) * vec3(ivec3(size.x, 1, size.y)));
	m[1] = GetMaterialColor(int(materials[1]), vec3(UV.x, height, UV.y) * vec3(ivec3(size.x, 1, size.y)));
	m[2] = GetMaterialColor(int(materials[2]), vec3(UV.x, height, UV.y) * vec3(ivec3(size.x, 1, size.y)));
	m[3] = GetMaterialColor(int(materials[3]), vec3(UV.x, height, UV.y) * vec3(ivec3(size.x, 1, size.y)));

	ivec2 c;
	c.x = materialOffset.x > 0.5 ? 1 : 0;
	c.y = materialOffset.y > 0.5 ? 2: 0;

	int s = c.x + c.y;

	ALBEDO = m[s];
}

//void light() {
//	// Called for every pixel for every light affecting the material.
//	// Uncomment to replace the default light processing function with this one.
//}
