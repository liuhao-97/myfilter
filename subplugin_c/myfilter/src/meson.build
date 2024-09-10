# src/meson.build

# Source files for the plugin
src_files = [
  'gstdatareschedule.c',
]

# Create the shared library (plugin)
library('gstdatareschedule',
  src_files,
  dependencies: [gst_dep, gstbase_dep],
  install: true,
  install_dir: join_paths(get_option('libdir'), 'gstreamer-1.0')
)