#version 450

#define NR_POINTS 16

layout (vertices = 2) out;

in vsData {
	float pointImportance;
} vsOut[];

out tessVsData {
	float pointImportance;
} tessOut[];

void main(){

	gl_TessLevelOuter[0] = 1;
	gl_TessLevelOuter[1] = NR_POINTS;

	tessOut[gl_InvocationID].pointImportance = vsOut[gl_InvocationID].pointImportance;


	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}