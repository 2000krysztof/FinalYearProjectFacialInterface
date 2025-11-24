#version 330 core
out vec4 fragColor;
in vec3 fragNormal;


void main() {
	vec3 N = normalize(fragNormal);

    float light = max(dot(N, normalize(vec3(0.4, 0.7, 0.2))), 0.0);
    float light_two = max(dot(N, normalize(vec3(-0.4, 0.4, 0.2))), 0.0)*0.1;
    float light_three = max(dot(N, normalize(vec3(-0.4, -0.3, 0.2))), 0.0)*0.1;

    fragColor = vec4(vec3(light+light_two+light_three), 1.0);
}
