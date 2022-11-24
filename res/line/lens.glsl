#include "/globals.glsl"

uniform vec2 lensPosition;
uniform float lensRadius;
uniform float lensDisp;
uniform float prevLensDisp;
uniform float lensDepthValue;
uniform vec2 delayedLensPosition;
uniform float time;


// Returns distance to given lens position
float distanceToLens(vec4 point, vec2 lensPos) {
	float aspectRatio = viewportSize.x/viewportSize.y;

#ifdef LENS_DEPTH
	vec3 lPos = vec3(lensPos, lensDepthValue);

	point.x *= aspectRatio;
	lPos.x *= aspectRatio;

	float dist = point.z > lensDepthValue ? distance(lPos.xy, point.xy) : distance(lPos, point.xyz);

#else
	vec2 lPos = lensPos;

	point.x *= aspectRatio;
	lPos.x *= aspectRatio;

	float dist = distance(lPos, point.xy);

#endif

	return dist;
}


// Displace a point
float disp(inout vec4 pos, vec2 lensPos) {

	float dist = distanceToLens(pos, lensPos);
	vec2 normDir = normalize(pos.xy - lensPos);

	float weight = 1.0 - smoothstep(0.0, lensRadius, dist);
	weight *= (lensRadius*viewportSize.y) * mix(prevLensDisp, lensDisp, easeOutElastic(time*2));

	pos.xy +=  weight * (normDir/viewportSize);	
	return weight;
}


vec4 displace(vec4 pos, float vertexImportance){
	
	// Interpolate between delayedLensPosition and lensPosition

	vec4 position = pos;


	position.z = vertexImportance;

	disp(position, lensPosition);
	position.z = 0;

	return position;
}


#define MAX_POINTS 64

// Lazy tesselation that returns how many points that are displaced < 64
int lazyTesselation(vec4 p1, vec4 p2) {
	int totalDisplacedPoints = 0;
	for(float t = 0.0; t <= 1.0; t += (1.0/float(MAX_POINTS) )) {
		vec4 currPoint = mix(p1, p2, t);
		bool isDisplaced = disp(currPoint, lensPosition) > 0.0;
		if(isDisplaced) {
			return MAX_POINTS;
		}
	}

	// GPU already limits the amount of tesselated control points to 64 (Nividia 1070)
	//return max(2, min(totalDisplacedPoints, MAX_POINTS));
	return 2;
}

// Finds entry, exit of line thorugh circle, similar to ray marchiung?
vec2 lensIntersection(vec4 p1, vec4 p2){
	float entry;
	float exit;
	bool entrySet = false;
	for(float t = 0.0; t < 1.0; t += 1.0/MAX_POINTS){
		vec4 currentPoint = mix(p1, p2, t);
		float dist = distanceToLens(currentPoint, lensPosition);
		if(dist < lensRadius && !entrySet){
			entry = t;
			entrySet = true;
		}
		if(dist > lensRadius && entrySet){
			exit = t;
			return vec2(entry, exit);
		}
	}

	if(entrySet){
		return vec2(entry, 1.0);
	}
	return vec2(0.0, 1.0);
	
}

// Project linear t-values onto a stepping function defined by lens radius
float projection(float t, float entry, float exit){
	float v = (smoothstep(0.0, 0.0, t) * entry) + t*(exit-entry) + step(1.0, t);
	return clamp(v, 0.0, 1.0);

}

// Finds the correct T-Value when projected onto custom function
float calcT(vec4 p1, vec4 p2, float t){
	vec2 entryExit = lensIntersection(p1, p2);
	float value = projection(t, entryExit.x, entryExit.y);
	return value;
}


// Recalulates the T values used for interpolation 
vec3 calulateInterpolationValues(vec4 p1, vec4 p2, float t){
	
	float prev, curr, next;

	float du = 1.0/MAX_POINTS;

	int vertIndex = int(round(t*MAX_POINTS));

	float t0 = t-du;
	float t1 = t;
	float t2 = t+du;


	prev = calcT(p1, p2, t0);
	curr = calcT(p1, p2, t1);
	next = calcT(p1, p2, t2);

	return vec3(prev, curr, next);
}

