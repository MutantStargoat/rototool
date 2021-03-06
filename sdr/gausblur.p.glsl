uniform sampler2D tex;
uniform vec2 pixsz;
uniform float stddev;
uniform int ksz;

#define SQRT_2PI	2.506628274631
float gaussian(float x, float sd)
{
	return exp(-(x * x) / (2.0 * sd * sd)) / (SQRT_2PI * sd);
}

void main()
{
	vec3 color = vec3(0.0, 0.0, 0.0);

	float sum = 0.0;
	for(int i=0; i<ksz; i++) {
		float x = float(i) - float(ksz) / 2.0;
		float g = gaussian(x, stddev);
		sum += g;

#ifdef HORIZ
		color += texture2D(tex, gl_TexCoord[0].st + vec2(x * pixsz.x, 0.0)).rgb * g;
#endif
#ifdef VERT
		color += texture2D(tex, gl_TexCoord[0].st + vec2(0.0, x * pixsz.y)).rgb * g;
#endif
	}
	color *= 1.0 / sum;

	gl_FragColor.rgb = color;
	gl_FragColor.a = 1.0;
}
