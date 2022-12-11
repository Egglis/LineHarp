#version 450
#define NR_POINTS 16
#include "/defines.glsl"
layout (vertices = 2) out;

in vsData {
	float pointImportance;
} vsOut[];

out tessVsData {
	float pointImportance;
} tessOut[];

uniform mat4 modelViewProjectionMatrix;

uniform vec2 viewportSize;
patch out int totalPoints;
patch out vec4 pp0;
patch out vec4 pp3;
patch out float del;

patch out float imp_p0;
patch out float imp_p3;

#include "/lens.glsl"



void main(){
    
	if(gl_InvocationID == 0) {

                pp0 = modelViewProjectionMatrix * gl_in[0].gl_Position;
                pp3 = modelViewProjectionMatrix * gl_in[3].gl_Position;
                
                vec4 p1 = modelViewProjectionMatrix * gl_in[1].gl_Position;
                vec4 p2 = modelViewProjectionMatrix * gl_in[2].gl_Position;

                int nr;

                #ifdef LENS_FEATURE
                    nr = 64;
                #else
                    nr = 2;
                #endif

                totalPoints = nr;


                imp_p0 = vsOut[0].pointImportance;
                imp_p3 = vsOut[3].pointImportance;

                gl_TessLevelOuter[0] = float(1);
                gl_TessLevelOuter[1] = float(nr);



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