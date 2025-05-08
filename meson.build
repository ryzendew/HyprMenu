project('hyprmenu', 'c',
  version: '0.1.0',
  license: 'MIT',
  default_options: [
    'c_std=c11',
    'warning_level=2',
    'prefix=/usr',
  ],
)

# Dependencies
gtk_dep = dependency('gtk4')
layer_shell_dep = dependency('gtk4-layer-shell-0')
glib_dep = dependency('glib-2.0')
gio_dep = dependency('gio-2.0')
gio_unix_dep = dependency('gio-unix-2.0')

# Installation directories
prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
datadir = join_paths(prefix, get_option('datadir'))
sysconfdir = get_option('sysconfdir')
icondir = join_paths(datadir, 'icons/hicolor')

# Source files
sources = [
  'src/main.c',
  'src/window.c',
  'src/app_grid.c',
  'src/category_list.c',
  'src/app_entry.c',
  'src/recent_apps.c',
  'src/config.c',
]

# Header files for installation
headers = [
  'src/window.h',
  'src/app_grid.h',
  'src/category_list.h',
  'src/app_entry.h',
  'src/recent_apps.h',
  'src/config.h',
]

# Build configuration
conf = configuration_data()
conf.set_quoted('PACKAGE_VERSION', meson.project_version())
conf.set_quoted('SYSCONFDIR', sysconfdir)
conf.set_quoted('DATADIR', datadir)

configure_file(
  output: 'config.h',
  configuration: conf
)

# Build the executable
executable('hyprmenu',
  sources,
  dependencies: [
    gtk_dep,
    layer_shell_dep,
    glib_dep,
    gio_dep,
    gio_unix_dep,
  ],
  install: true,
  install_dir: bindir
)

# Install header files
install_headers(headers)

# Install desktop file
install_data('data/org.hyprmenu.desktop',
  install_dir: join_paths(datadir, 'applications')
)

# Install icons
install_data('data/icons/hyprmenu.svg',
  install_dir: join_paths(icondir, 'scalable/apps')
)

# Install license and documentation
install_data('LICENSE', install_dir: join_paths(datadir, 'licenses', 'hyprmenu'))
install_data('README.md', install_dir: join_paths(datadir, 'doc', 'hyprmenu')) 