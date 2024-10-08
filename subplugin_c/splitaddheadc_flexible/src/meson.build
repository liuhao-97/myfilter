# src/meson.build

# Source files for the plugin
src_files = [
  'gstsplitaddheadc.c',
]

# Create the shared library (plugin)
library('gstsplitaddheadc',
  src_files,
  dependencies: [gst_dep, gstbase_dep],
  install: true,
  install_dir: join_paths(get_option('libdir'), 'gstreamer-1.0')
)