shader_type spatial;
render_mode skip_vertex_transform, cull_back, blend_mix, depth_draw_always;
//render_mode skip_vertex_transform, cull_disabled, blend_mix, depth_draw_always, unshaded;

//uniform mat4 bbones[41];
uniform sampler2D tex;
//uniform int bonesCount;

uniform sampler2D animtex: filter_nearest;


varying flat vec4 CCCC;
varying flat ivec4 BI;
varying vec4 BW;

mat4 GetBone(int id)
{
	mat4 ret = mat4(1);
	for (int i=0; i<4; ++i)
		ret[i].xyz = texelFetch(animtex, ivec2(id, i), 0).xyz;
	return ret;
}

void vertex() {
	ivec4 bi = ivec4(CUSTOM0*255.0);//-ivec4(8192)) / (ivec4(1024) / ivec4(31));

	mat4 mats = GetBone(int(TIME/10.0)%40);

	if (true) {
		mats = GetBone(bi[0]);
	}



	vec3 VERT = VERTEX;

	vec4 bw = CUSTOM1;//vec4(unpackUnorm2x16(BONE_WEIGHTS.x), unpackUnorm2x16(BONE_WEIGHTS.y));

	if (true) {
		const int C_W = 1;

		mats = mat4(0);

		//VERT = vec3(0.0);
		float sum = 0.0;
		for (int i=0; i<C_W; ++i) {
			sum += bw[i];
			mats += GetBone(bi[i]) * bw[i];
		//	VERT += ((GetBone(bi[i]) * vec4(VERTEX, 1))).xyz * bw[i];
		}
		mats /= sum;
		//VERT = VERT / sum;
	}

		//mats = mat4(1);



	VERTEX = (MODELVIEW_MATRIX * (mats * vec4(VERT, 1))).xyz;
	NORMAL = (MODELVIEW_MATRIX * (mats * vec4(NORMAL, 0))).xyz;
	//BINORMAL = (MODELVIEW_MATRIX * (mats * vec4(BINORMAL, 0))).xyz;
	//TANGENT = (MODELVIEW_MATRIX * (mats * vec4(TANGENT, 0))).xyz;
	//VERTEX = (MODELVIEW_MATRIX * vec4(VERT, 1)).xyz;
	// Called for every vertex the material is visible on.

	COLOR = vec4(0,0,0,1);
	COLOR.w = float(bi.x);
	bw = CUSTOM1;
	COLOR.w = (abs(bw.x) + abs(bw.y) + abs(bw.z) + abs(bw.w)) * 256.0*256.0*256.0;
	COLOR.w = float(int(bi.x/* + bi.y + bi.z + bi.w*/));// / float(bonesCount*100);
	CCCC = COLOR/1164.0;
	BW = CUSTOM1;
	BI = ivec4(CUSTOM0*255.0);
}

void fragment() {
	vec4 col = texture(tex, UV);
	ALBEDO = col.xyz;
	//ALBEDO = vec3(CCCC.w);
	ALPHA = 1.0;



	int x = int(FRAGCOORD.x)%8;
	int y = int(FRAGCOORD.y)%4;

	int comp = 0;//y/4;




	vec3 borderColors[4] = {
		vec3(1, 0, 0),
		vec3(1, 0.15, 0.15),
		vec3(0.15, 1, 0.15),
		vec3(0.15, 0.15, 1)};
	vec3 color = borderColors[comp];

	if (x < 32) {
		int bit = x + (y*8);

		int value = BI[comp] & (1<<bit);

		if (value > 0) {
			color = vec3(1.0);
		} else {
			color = vec3(0.0);
		}



	}



	//unpackHalf2x16(BW.x)


	//ALBEDO = BI.xxx;//color;
	//ALBEDO = color;






	//ALPHA = COLOR.w;
	//ALBEDO = COLOR.xyz;
	// Called for every pixel the material is visible on.
}

//void light() {
	// Called for every pixel for every light affecting the material.
	// Uncomment to replace the default light processing function with this one.
//}
