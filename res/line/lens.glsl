
uniform vec2 lensPosition;
uniform float lensRadius;
uniform float lensDisp;
uniform float lensDepthValue;

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
vec4 disp(vec4 pos, vec2 lensPos) {

	float dist = distanceToLens(pos, lensPosition);
	vec2 normDir = normalize(pos.xy - lensPosition);

	float weight = 1.0f - smoothstep(0.0, lensRadius, dist);
	weight *= (lensRadius*viewportSize.y)*lensDisp;

	pos.xy +=  weight * (normDir/viewportSize);	
	return pos;
}


vec4 displace(vec4 pos, float vertexImportance){
	pos.z = vertexImportance;
	return disp(pos, lensPosition); 
};