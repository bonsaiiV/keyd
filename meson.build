project('keyd', 'c',
  version : '2.5.0',
  default_options : ['warning_level=3', 'c_std=c11'],
)

conf_data = configuration_data()
conf_data.set('SOCKET_PATH', '"/@0@"'.format(get_option('socket_path')))
conf_data.set('CONFIG_DIR', '"/@0@/keyd"'.format(get_option('sysconfdir')))
conf_data.set('DATA_DIR', '"/@0@/@1@/keyd"'.format(get_option('prefix'), get_option('datadir')))
conf_data.set('VERSION', '"@0@"'.format(meson.project_version()))
conf_data.set('_FORTIFY_SOURCE', 2)
#conf_data.set('_DEFAULT_SOURCE', true)
#conf_data.set('NDEBUG', true)

configure_file(
  output : 'projconf.h',
  configuration : conf_data
)



include_dir = include_directories('include', '.')
subdir('keyd')

#tests are in subfolder
subdir('test')

#docs
scdoc = dependency('scdoc', version: '>=1.9.2', native: true, required: false)
if scdoc.found()
	scdoc_prog = find_program(scdoc.get_variable(pkgconfig: 'scdoc'), native: true)
	mandir = get_option('mandir')
	man_files = [
		'docs/keyd.scdoc',
		'docs/keyd-application-mapper.scdoc',
	]
	foreach filename : man_files
		topic = filename.split('/')[-1].split('.')[0]
		output = '@0@.1.gz'.format(topic)

		custom_target(
			output,
			input: filename,
			output: output,
			command: scdoc_prog,
			install: true,
			feed: true,
			capture: true,
			install_dir: '@0@/man1'.format(mandir)
		)
	endforeach
endif
