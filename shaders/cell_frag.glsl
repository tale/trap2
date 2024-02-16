#version 330 core

in vec2 tex_coords;
flat in vec4 fg;
flat in vec4 bg;

layout(location = 0, index = 0) out vec4 color;
layout(location = 0, index = 1) out vec4 a_mask;

uniform int pass;
uniform sampler2D atlas;

// out vec4 frag_color;

// uniform vec4 text_color;
// uniform vec4 bg_color;
// uniform sampler2D text;

void main() {
	// vec4 sampled = vec4(text_color.rgb, texture(text, tex_coords).r);
	// frag_color = mix(bg_color, sampled, sampled.a);


	// Background rendering pass
	if (pass == 1) {
		if (bg.a == 0.0) {
			discard;
		}

		a_mask = vec4(1.0);
		color = vec4(bg.rgb * bg.a, bg.a);
		return;
	}

	vec3 text_color = texture(atlas, tex_coords).rgb;
	a_mask = vec4(text_color, text_color.r);
	color = vec4(fg.rgb, 1.0);
}
