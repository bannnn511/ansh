project('ansh', 'c',
        version : '1.0.0',
        default_options : ['warning_level=3', 'c_std=c11'])

ansh = executable('ansh', 'main.c','parse.c', install : true)

test('test', ansh)