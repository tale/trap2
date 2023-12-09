#version 330 core

in vec2 tex_coords;
out vec4 frag_color;

uniform vec4 text_color;
uniform sampler2D text;

void main() {
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, tex_coords).r);
	frag_color = vec4(text_color) * sampled;
}
