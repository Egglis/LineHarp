#version 450

#define NR_POINTS 16

layout (vertices = 2) out;

in vsData {
	float pointImportance;
} vsOut[];

out tessVsData {
	float pointImportance;
    float tDiff;
} tessOut[];

uniform mat4 modelViewProjectionMatrix;

patch out vec4 pp0;
patch out vec4 pp3;



void main(){

	gl_TessLevelOuter[0] = 1;
	gl_TessLevelOuter[1] = NR_POINTS;
    tessOut[gl_InvocationID].tDiff = 1.0f/NR_POINTS;

	// Should do interpolation between 0-1 to find point importance of "new" vertecies
	// tessOut[gl_InvocationID].pointImportance = vsOut[gl_InvocationID].pointImportance;
	tessOut[gl_InvocationID].pointImportance = vsOut[gl_InvocationID].pointImportance;
	//tessOut[gl_InvocationID].pointImportance = vsOut[gl_InvocationID].pointImportance;

	if(gl_InvocationID == 0) {
                gl_TessLevelOuter[0] = float(1);
                gl_TessLevelOuter[1] = float(16);

                pp0 = modelViewProjectionMatrix * gl_in[0].gl_Position;
                pp3 = modelViewProjectionMatrix * gl_in[3].gl_Position;
        }

        if(gl_InvocationID == 0) {
                gl_out[gl_InvocationID].gl_Position = modelViewProjectionMatrix * gl_in[1].gl_Position;
        }

        if(gl_InvocationID == 1) {
                gl_out[gl_InvocationID].gl_Position = modelViewProjectionMatrix * gl_in[2].gl_Position;
        }
}