#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 fragPos;
uniform float uTime;
uniform vec2 uResolution;

void main(){
	vec3 color;
	float fragDistanceToScreenCenter, timeOffset = uTime;
	for(int i = 0; i < 3; i++){
		vec2 uv, normFragPos = gl_FragCoord.xy/uResolution;
		vec3 p = fragPos * 0.5;
		uv = normFragPos;
		normFragPos -= 0.5f;
		normFragPos.x *= uResolution.x/uResolution.y;
		timeOffset += .07;
		fragDistanceToScreenCenter = length(normFragPos);
		uv += normFragPos/fragDistanceToScreenCenter * (sin(timeOffset) + 1.) * abs(sin(fragDistanceToScreenCenter * 9. - timeOffset - timeOffset));
		color[i] = .01/length(mod(uv, 1.)- .5);
		color += p;
	}

	FragColor = vec4(color/fragDistanceToScreenCenter, uTime);
}