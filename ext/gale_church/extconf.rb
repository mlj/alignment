require 'mkmf'
extension_name = 'gale_church'
dir_config(extension_name)
$CFLAGS='-Wall'
create_makefile(extension_name)
