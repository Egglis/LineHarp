#version 450

#define NR_POINTS 8

layout (vertices = 2) out;

in vsData {
	float pointImportance;
} vsOut[];

out tessVsData {
	float pointImportance;
} tessOut[];

patch out vec4 p0;
patch out vec4 p3;

void main(){

	gl_TessLevelOuter[0] = 1;
	gl_TessLevelOuter[1] = NR_POINTS;

	// Should do interpolation between 0-1 to find point importance of "new" vertecies
	// tessOut[gl_InvocationID].pointImportance = vsOut[gl_InvocationID].pointImportance;
	tessOut[gl_InvocationID].pointImportance = vsOut[gl_InvocationID].pointImportance;
	//tessOut[gl_InvocationID].pointImportance = vsOut[gl_InvocationID].pointImportance;

	if(gl_InvocationID == 0) {
                gl_TessLevelOuter[0] = float(1);
                gl_TessLevelOuter[1] = float(16);

                p0 = gl_in[0].gl_Position;
                p3 = gl_in[3].gl_Position;
        }

        if(gl_InvocationID == 0) {
                gl_out[gl_InvocationID].gl_Position = gl_in[1].gl_Position;
        }

        if(gl_InvocationID == 1) {
                gl_out[gl_InvocationID].gl_Position = gl_in[2].gl_Position;
        }
}