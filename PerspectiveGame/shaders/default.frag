#version 330 core

 in vec3 fragPos;
 in vec3 fragColor;
 in vec3 fragNormal;
 in vec2 fragTexCoord;
 in float shininess;
 in float Ks;
 in float Kn;
 in vec3 lightPos;
 in vec3 camPos;

out vec4 outColor;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float inBlendAmount;
const vec3 portalTint = vec3(.5, .5, .5);

const vec3 lightColor = vec3(1.0, 0.9, 0.9);
const float lightPower = 40.0;
vec3 diffuseColor = fragColor;
vec3 ambientColor = fragColor;
const vec3 specColor = vec3(1.0, 0.9, 0.9);
const float screenGamma = 2.2; // Assume the monitor is calibrated to the sRGB color space

vec4 phong(vec4 color) {
	ambientColor = vec3(color);
	diffuseColor = ambientColor;

	vec3 normal = normalize(fragNormal);
    vec3 lightDir = lightPos - fragPos;
   	float distance = length(lightDir);
   	distance = distance * distance;
   	lightDir = normalize(lightDir);

   	float lambertian = max(dot(lightDir, normal), 0.0);
   	float specular = 0.0;

  	if (lambertian > 0.0) {
    	// orthographic:
    	vec3 viewDir = camPos;

   		vec3 halfDir = normalize(lightDir + viewDir);
   		float specAngle = max(dot(halfDir, normal), 0.0);
   		specular = pow(specAngle, 50); // 50 is shininess, make this better!
	}	

  	vec3 colorLinear = ambientColor * 0.05 +
        Ks * diffuseColor * lambertian * lightColor * lightPower / distance +
        Kn * specColor * specular * lightColor * lightPower / distance;
  
  	// use the gamma corrected color in the fragment
	return vec4(colorLinear, 1);
}

void main() {
	 //outColor = mix(texture(texture1, fragTexCoord), texture(texture2, fragTexCoord), 0.2);
	 //outColor = texture(texture2, fragTexCoord);
	 //outColor = vec4(fragColor, 1);
     vec3 c = vec3(phong(vec4(fragColor, 1)));
     outColor = vec4(mix(c, portalTint, inBlendAmount), 1);
};