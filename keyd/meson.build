sources = [
  'config.c',
  'config.c',
  'daemon.c',
  'device.c',
  'evloop.c',
  'ini.c',
  'ipc.c',
  'keyboard.c',
  'keyd.c',
  'keys.c',
  'log.c',
  'macro.c',
  'monitor.c',
  'strutil.c',
  'unicode.c',
  'util.c',
  'vkbd/@0@.c'.format(get_option('vkbd')),
]
strutil_dep = declare_dependency(sources : 'strutil.c')
executable('keyd',
  sources,
  include_directories : include_dir,
  install : true,
  install_dir : get_option('bindir'),
)

