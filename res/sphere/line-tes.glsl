#version 450

layout (isolines, equal_spacing) in;

in tessVsData {
	float pointImportance;
} tessOut[];

out vsData {
	float pointImportance;
	vec3 prev;
	vec3 next;
	vec4 up;
	vec4 down;

} vsOut;

patch in vec4 p0;
patch in vec4 p3;

uniform float lineWidth;

vec4 catmull(vec4 p0, vec4 p1, vec4 p2, vec4 p3, float u ){
	float b0 = (-1.f * u) + (2.f * u * u) + (-1.f * u * u * u);
	float b1 = (2.f) + (-5.f * u * u) + (3.f * u * u * u);
	float b2 = (u) + (4.f * u * u) + (-3.f * u * u * u);
	float b3 = (-1.f * u * u) + (u * u * u);
	float m = 0.5f;
	vec4 new_pos = m * (b0*p0 + b1*p1 + b2*p2 + b3*p3);
	
	float b0_d = -1.f + 4 * u - 3 * u*u;
	float b1_d = 9 * u*u - 10*  u;
	float b2_d = 1 + 8 * u - 9 * u*u;
	float b3_d = -2 * u + 3 * u*u;

	vec4 tangent = normalize(b0_d*p0 + b1_d*p1 + b2_d*p2 + b3_d*p3);
	vec3 up = vec3(1, 0, 0);
	vec3 normal = cross(up, tangent.xyz);

	vsOut.up = new_pos ;
	vsOut.down = new_pos + vec4(1,0,0,1);

	vsOut.up.z = 0;
	vsOut.down.z = 0;

	return vec4(new_pos.x, new_pos.y, 0, 1);
}

void main(){

	vsOut.prev = p0.xyz;
	vsOut.next = p3.xyz;

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	float du = u/16;

	vec4 p1 = gl_in[0].gl_Position;
	vec4 p2 = gl_in[1].gl_Position;

    float slope = (p2.y - p1.y) / (p2.x - p1.x);
    float x = ((p2.x - p1.x) * u) + p1.x;
    float y = (u * slope * (p2.x - p1.x)) + p1.y;
	
	// Importance is interpolated
	vsOut.pointImportance = mix(tessOut[0].pointImportance, tessOut[0].pointImportance, u);

	vsOut.prev = catmull(p0, p1, p2, p3, u).xyz;
	vsOut.next = catmull(p0, p1, p2, p3, u).xyz;

    gl_Position = catmull(p0, p1, p2, p3, u);


}