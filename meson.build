project('trap2', 'c', default_options: [
  'c_std=c11',
  'default_library=static'
])

# Honestly can Apple just shut up about the deprecation warnings
add_global_arguments('-DGL_SILENCE_DEPRECATION', language: 'c')
lib_dir = meson.current_source_dir() + '/subprojects/vterm/.libs'

sources = [
  'src/main.c',
  # 'src/shader.c',
  'src/font.c',
  'src/term/init.c',
  'src/term/destroy.c',
  'src/term/handle.c',
  'src/term/render.c',
  'src/term/resize.c'
]

run_command('./configure.sh', check: true)
message('Built vendor dependencies')

executable('trap2', sources,
  include_directories: ['include'],
  dependencies: [
    dependency('freetype2', required: true),
    dependency('sdl2', required: true),
    dependency('glew', required: true),
    dependency('OpenGL', required: true),
    declare_dependency(
      link_args: ['-L' + lib_dir, '-lvterm'],
      include_directories: ['subprojects/vterm/include']
    )
  ]
)
