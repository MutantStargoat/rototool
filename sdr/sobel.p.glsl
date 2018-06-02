uniform sampler2D tex;
uniform vec2 pixsz;

float rgb2gray(vec4 c)
{
	return c.r * 0.3 + c.g * 0.59 + c.b * 0.11;
}

void main()
{
	vec2 uv = gl_TexCoord[0].st;

	float dx = 0.0;
	dx += rgb2gray(texture2D(tex, uv + vec2(-pixsz.x, -pixsz.y)));
	dx -= rgb2gray(texture2D(tex, uv + vec2(pixsz.x, -pixsz.y)));
	dx += rgb2gray(texture2D(tex, uv + vec2(-pixsz.x, 0.0))) * 2.0;
	dx -= rgb2gray(texture2D(tex, uv + vec2(pixsz.x, 0.0))) * 2.0;
	dx += rgb2gray(texture2D(tex, uv + vec2(-pixsz.x, pixsz.y)));
	dx -= rgb2gray(texture2D(tex, uv + vec2(pixsz.x, pixsz.y)));

	float dy = 0.0;
	dy += rgb2gray(texture2D(tex, uv + vec2(-pixsz.x, -pixsz.y)));
	dy -= rgb2gray(texture2D(tex, uv + vec2(-pixsz.x, pixsz.y)));
	dy += rgb2gray(texture2D(tex, uv + vec2(0.0, -pixsz.y))) * 2.0;
	dy -= rgb2gray(texture2D(tex, uv + vec2(0.0, pixsz.y))) * 2.0;
	dy += rgb2gray(texture2D(tex, uv + vec2(pixsz.x, -pixsz.y)));
	dy -= rgb2gray(texture2D(tex, uv + vec2(pixsz.x, pixsz.y)));

	float value = sqrt(dx * dx + dy * dy);

	gl_FragColor.rgb = vec3(value, value, value);
	gl_FragColor.a = 1.0;
}
