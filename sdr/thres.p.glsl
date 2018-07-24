uniform sampler2D tex;
uniform float thres;
uniform float smooth;
uniform float invsign;

void main()
{
	vec4 pixel = texture2D(tex, gl_TexCoord[0].st);
	float value = dot(pixel.rgb, vec3(0.3, 0.59, 0.11));

	value = smoothstep(thres - smooth / 2.0, thres + smooth / 2.0, value);
	value = ((value - 0.5) * invsign) + 0.5;

	gl_FragColor.rgb = vec3(value, value, value);
	gl_FragColor.a = 1.0;
}
