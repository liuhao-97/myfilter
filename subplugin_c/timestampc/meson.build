# Root meson.build
project('timestampc', 'c', 'cpp',
  version: '1.0',
  license: ['LGPL-2.1'],
  meson_version: '>=0.50.0',
  default_options: [
    'werror=false',
    'warning_level=2',
    'c_std=gnu89',
    'cpp_std=c++17',
    'b_lundef=false'
  ]
  )

# Find GStreamer dependencies
gst_dep = dependency('gstreamer-1.0')
gstbase_dep = dependency('gstreamer-base-1.0')

# Process the src folder
subdir('src')