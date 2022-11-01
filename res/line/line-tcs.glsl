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
patch out int totalPoints;
patch out float imp_p0;
patch out float imp_p3;


void main(){
    
    // TODO Tesselated based on magic lens 

	gl_TessLevelOuter[0] = 1;
	gl_TessLevelOuter[1] = NR_POINTS;

    totalPoints = NR_POINTS;

	if(gl_InvocationID == 0) {
                gl_TessLevelOuter[0] = float(1);
                gl_TessLevelOuter[1] = float(16);

                pp0 = modelViewProjectionMatrix * gl_in[0].gl_Position;
                pp3 = modelViewProjectionMatrix * gl_in[3].gl_Position;

                imp_p0 = vsOut[0].pointImportance;
                imp_p3 = vsOut[3].pointImportance;

        }

    if(gl_InvocationID == 0) {
        gl_out[gl_InvocationID].gl_Position = modelViewProjectionMatrix * gl_in[1].gl_Position;
        tessOut[gl_InvocationID].pointImportance = vsOut[1].pointImportance;

    }

    if(gl_InvocationID == 1) {
        gl_out[gl_InvocationID].gl_Position = modelViewProjectionMatrix * gl_in[2].gl_Position;
        tessOut[gl_InvocationID].pointImportance = vsOut[2].pointImportance;

    }
}