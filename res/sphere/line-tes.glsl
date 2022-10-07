#version 450

layout (isolines, equal_spacing, ccw) in;

in tessVsData {
	float pointImportance;
} tessOut[];

out vsData {
	float pointImportance;
	vec3 prev;
	vec3 next;
} vsOut;

void main(){

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	
	vec3 start = vec3(gl_in[0].gl_Position);
	vec3 end = vec3(gl_in[1].gl_Position);

	// Move to globals.glsl or similar
	int num_points = 16;

	// Normalized length between segments
	float du = 1/num_points;


	vsOut.pointImportance = tessOut[0].pointImportance;
	vec3 prev;
	vec3 next;

	// Handle displacment of line segments and also calculate the (prev, next) here
	// Also do importance interpolation using mix()
	// Implement bezier curve

	prev = vec3(mix(start.x, end.x, u), mix(start.y, end.y, u), 0);
	next = vec3(mix(start.x, end.x, u), mix(start.y, end.y, u), 0);

	vsOut.prev = prev;
	vsOut.next = next;



	gl_Position = vec4(mix(start.x, end.x, u), mix(start.y, end.y, u), 0, 1);
}