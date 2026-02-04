echo loading .gdbinit from: ${PROJECT_ROOT}/.runtime.gdbinit \n

file ~/git/c/42london/webserv/webserv
br main
set follow-exec-mode same
set trace-commands on
set detach-on-fork on
#catch fork
catch exec