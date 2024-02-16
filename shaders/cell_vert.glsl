#version 330 core

layout(location = 0) in vec2 grid_coords;
layout(location = 1) in vec4 glyph;
layout(location = 2) in vec4 uv;
layout(location = 3) in vec4 color;
layout(location = 4) in vec4 bg_color;

out vec2 tex_coords;
flat out vec4 fg;
flat out vec4 bg;

uniform mat4 projection;
uniform vec2 cell_size;
uniform int pass;

void main() {
	// gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
	// tex_coords = vertex.zw;
	
	vec2 pos;
	pos.x = (gl_VertexID == 0 || gl_VertexID == 1) ? 1. : 0.;
	pos.y = (gl_VertexID == 0 || gl_VertexID == 3) ? 0. : 1.;

	vec2 cell_pos = grid_coords * cell_size;

	fg = vec4(color.rgb / 255.0, 1.0); // TODO: Unhardcode
	bg = bg_color / 255.0;

	// Background rendering pass
	if (pass == 1) {
		gl_Position = projection * vec4(cell_pos * cell_size, 0.0, 1.0);
		tex_coords = vec2(0, 0);
	} else {
		vec2 glyph_size = glyph.zw;
		vec2 glyph_offset = glyph.xy;
		glyph_offset.y = cell_size.y - glyph_offset.y;

		vec2 fin = cell_pos + glyph_size * pos + glyph_offset;
		gl_Position = projection * vec4(fin, 0.0, 1.0);

		vec2 uv_offset = uv.xy;
		vec2 uv_size = uv.zw;
		tex_coords = uv_offset + pos * uv_size;
	}
}
