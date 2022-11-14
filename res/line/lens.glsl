
#include "/globals.glsl"

uniform vec2 lensPosition;
uniform float lensRadius;
uniform float lensDisp;
uniform float lensDepthValue;
uniform vec2 delayedLensPosition;
uniform float time;
uniform float actionStart;
uniform float actionEnd;
uniform vec2[10] dlp; 



// Returns distance to given lens position
float distanceToLens(vec4 point, vec2 lensPos) {
	float aspectRatio = viewportSize.x/viewportSize.y;

#ifdef LENS_DEPTH
	vec3 lPos = vec3(lensPos, lensDepthValue);

	point.x *= aspectRatio;
	lPos.x *= aspectRatio;

	float dist = distance(lPos, point.xyz);
#else
	vec2 lPos = lensPos;

	point.x *= aspectRatio;
	lPos.x *= aspectRatio;

	float dist = distance(lPos, point.xy);
#endif

	return dist;
}


// Displace a point
bool disp(inout vec4 pos, vec2 lensPos) {

	float dist = distanceToLens(pos, lensPos);
	vec2 normDir = normalize(pos.xy - lensPos);

	float weight = 1.0f - smoothstep(0.0, lensRadius, dist);
	//weight *= easeOutElastic(time);

	weight *= (lensRadius*viewportSize.y)*lensDisp;
	pos.xy +=  weight * (normDir/viewportSize);	
	return weight > 0.0f;
}


vec4 displace(vec4 pos, float vertexImportance){
	
	// Interpolate between delayedLensPosition and lensPosition

	vec4 position = pos;
	vec4 delayedPosition = pos;

	//vec2 dlLensPosition = m_delayedLensPosition * (1.0f - fT) + m_lensPosition * fT;

	position.z = vertexImportance;
	delayedPosition.z = vertexImportance;

	disp(position, lensPosition);
	disp(delayedPosition, delayedLensPosition);

	position = mix(position, delayedPosition, (1.0f-easeOutElastic(time)));


	position.z = 0;

	return position;
}


#define MAX_POINTS 64

// Lazy tesselation that returns how many points that are displaced < 64
int lazyTesselation(vec4 p1, vec4 p2) {
	int totalDisplacedPoints = 0;
	for(float t = 0.0f; t <= 1.0f; t += (1.0f/float(MAX_POINTS) )) {
		vec4 currPoint = mix(p1, p2, t);
		bool isDisplaced = disp(currPoint, lensPosition);
		if(isDisplaced) totalDisplacedPoints++;
	}

	// GPU already limits the amount of tesselated control points to 64 (Nividia 1070)
	//return max(2, min(totalDisplacedPoints, MAX_POINTS));
	return MAX_POINTS;
}