#version 450

layout (isolines, equal_spacing) in;

in tessVsData {
	float pointImportance;
	float tDiff;
} tessOut[];

out vsData {
	float pointImportance;
	vec2 prev;
	vec2 next;

} vsOut;

patch in vec4 pp0;
patch in vec4 pp3;

uniform float testSlider;
uniform vec2 viewportSize;


vec4 catmull(vec4 p0, vec4 p1, vec4 p2, vec4 p3, float u ){

	float alpha = 1.0;
    float tension = 0.0;
    
    float t01 = pow(distance(p0, p1), alpha);
	float t12 = pow(distance(p1, p2), alpha);
	float t23 = pow(distance(p2, p3), alpha);

	vec2 m1 = (1.0f - tension) *
    	(p2.xy - p1.xy + t12 * ((p1.xy - p0.xy) / t01 - (p2.xy - p0.xy) / (t01 + t12)));
	vec2 m2 = (1.0f - tension) *
    	(p2.xy - p1.xy + t12 * ((p3.xy - p2.xy) / t23 - (p3.xy - p1.xy) / (t12 + t23)));
    
	vec2 a = 2.0f * (p1.xy - p2.xy) + m1 + m2;
	vec2 b = -3.0f * (p1.xy - p2.xy)  - m1 - m1 - m2;
	vec2 c = m1;
	vec2 d = p1.xy;

	vec2 new_pos =  (a * u * u * u) + (b * u * u) + (c * u) + (d);

	return vec4(new_pos, 0, 1);

}

vec4 straightline(float u){
        vec4 p0 = gl_in[0].gl_Position;
        vec4 p1 = gl_in[1].gl_Position;

        float slope = (p1.y - p0.y) / (p1.x - p0.x);
        float x = ((p1.x - p0.x) * u) + p0.x;
        float y = (u * slope * (p1.x - p0.x)) + p0.y;

        return vec4(x, y, 0, 1);

}

void main(){

	
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec4 p0 = pp0;
	vec4 p1 = gl_in[0].gl_Position;
	vec4 p2 = gl_in[1].gl_Position;
	vec4 p3 = pp3;


	// Importance is interpolated
	vsOut.pointImportance = mix(tessOut[0].pointImportance, tessOut[0].pointImportance, u);

	float du = 1.0f/16;
	float t0 = u-du;
	float t1 = u;
	float t2 = u+du;

	vec4 prev_pos = catmull(p0, p1, p2, p3, t0);
	vec4 pos = catmull(p0, p1, p2, p3, t1);
	vec4 next_pos = catmull(p0, p1, p2, p3, t2);
	
	vsOut.prev = prev_pos.xy;
	vsOut.next = next_pos.xy;

    gl_Position = pos;


}