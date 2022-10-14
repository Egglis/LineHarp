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


uniform float xAxisScaling;
uniform float yAxisScaling;
uniform float testSlider;
uniform vec2 lensPosition;
uniform float lensRadius;
uniform vec2 viewportSize;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 inverseModelViewProjectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 inverseViewMatrix;

vec3 bezier(float u, vec3 p0, vec3 p1, vec3 p2, vec3 p3)
{
	float B0 = (1.-u)*(1.-u)*(1.-u);
	float B1 = 3.*u*(1.-u)*(1.-u);
	float B2 = 3.*u*u*(1.-u);
	float B3 = u*u*u;

	vec3 p = B0*p0 + B1*p1 + B2*p2 + B3*p3;
	return p;
} 

// Same solution as previously only now it effect all the "generated" vertexes as well
vec3 lens(vec3 p0, vec3 p3, float u){

	float aspectRatio = viewportSize.x/viewportSize.y;
	vec2 lPos = lensPosition;
	lPos.x *= aspectRatio;

	vec3 currPoint = mix(p0, p3, u);
	vec4 MPoint = modelViewProjectionMatrix*vec4(currPoint, 1);
	MPoint.x *= aspectRatio;

	float dl = distance(lPos, MPoint.xy);

	if (dl <= lensRadius) {
		vec2 dir = normalize(vec2(MPoint.x - lPos.x, MPoint.y - lPos.y));

		
		float zoomIntensity = 1;
		float x = dl/lensRadius;
		float scaling = zoomIntensity*cos(3.1415 + x*2*3.1415)+zoomIntensity;
		
		return currPoint+vec3(dir*scaling*testSlider, 0);

	}
	return mix(p0, p3, u);
}


vec3 bezierDisplacment(vec3 cp) {

	float aspectRatio = viewportSize.x/viewportSize.y;
	vec2 lPos = lensPosition;
	lPos.x *= aspectRatio;

	vec4 MPoint = modelViewProjectionMatrix*vec4(cp, 1);
	MPoint.x *= aspectRatio;

	float dl = distance(lPos, MPoint.xy);
	if(dl <= lensRadius) {
		vec2 dir = normalize(vec2(MPoint.x - lPos.x, MPoint.y - lPos.y));

		float zoomIntensity = 1;
		float x = dl/lensRadius;
		float scaling = zoomIntensity*cos(3.1415 + x*2*3.1415)+zoomIntensity;


		float pxlRadius = lensRadius*inverseModelViewProjectionMatrix[1].y;
		return vec3(dir, 0)*pxlRadius/2*scaling;
	}

	return vec3(0);
}


vec3 bezierLens(vec3 p0, vec3 p3, float u){
	
	vec3 cp0 = p0;
	vec3 cp3 = p3;
	vec3 cp1 = mix(p0, p3, 0.33);
	vec3 cp2 = mix(p0, p3, 0.66);


	cp0 += bezierDisplacment(cp0);
	cp3 += bezierDisplacment(cp3);

	cp1 += bezierDisplacment(cp1);
	cp2 += bezierDisplacment(cp2);



	return bezier(u, cp0, cp1, cp2, cp3);
}



void main(){
	// Move to globals.glsl or similar
	int num_points = 16;


	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	
	vsOut.pointImportance = mix(tessOut[0].pointImportance, tessOut[1].pointImportance, u);

	// Initial start and end
	vec3 p0 = vec3(gl_in[0].gl_Position);
	vec3 p3 = vec3(gl_in[1].gl_Position);

	// Construct 2 extra control points|
	vec3 p1 = vec3(mix(p0.x, p3.x, 1/3), mix(p0.y, p3.y, 1/3), 0);
	vec3 p2 = vec3(mix(p0.x, p3.x, 2/3), mix(p0.y, p3.y, 2/3), 0);

	// Check for displacment

	// Normalized length between segments
	float du = 1/num_points;

	vec3 prev;
	vec3 next;
	
	/*
	// Bezier
	vsOut.prev = bezierLens(p0,p3,u-du);
	vsOut.next = bezierLens(p0,p3,u+du);

	gl_Position = vec4(bezierLens(p0, p3, u) , 1);
	*/
	
	
	vsOut.prev = lens(p0,p3,u-du);
	vsOut.next = lens(p0,p3,u+du);

	gl_Position = vec4(lens(p0, p3, u) , 1);
	
	
}