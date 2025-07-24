# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    .gdbinit                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abelov <abelov@student.42london.com>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/23 21:56:35 by abelov            #+#    #+#              #
#    Updated: 2025/07/23 21:56:36 by abelov           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#layout reg
set disassemble-next-line on
set print static-members off
set print pretty on
set height unlimited
set pagination off
set confirm off
set max-value-size unlimited


directory ~/staging/glibc-2.31
directory ~/staging/glibc-2.31/elf
directory ~/staging/glibc-2.31/nptl
directory ~/staging/glibc-2.31/dlfcn
directory ~/staging/glibc-2.31/libio
directory ~/staging/glibc-2.31/misc
directory ~/staging/glibc-2.31/csu
directory ~/staging/glibc-2.31/ctype
directory ~/staging/glibc-2.31/stdlib
directory ~/staging/glibc-2.31/sysdeps
directory ~/staging/glibc-2.31/support
directory ~/staging/glibc-2.31/signal
directory ~/staging/glibc-2.31/string
directory ~/staging/glibc-2.31/stdio-common
directory ~/staging/libxext-1.3.4/src
directory ~/staging/libx11-1.6.9/src
directory ~/staging/gcc-9-9.4.0/src/libsanitizer/sanitizer_common
directory ~/staging/gcc-9-9.4.0/src/libsanitizer/asan
directory ~/staging/gcc-9-9.4.0/src/libsanitizer/ubsan
directory ~/staging/gcc-9-9.4.0/src/libsanitizer/ubsan
directory ~/staging/libsdl2-mixer-2.0.4+dfsg1
directory ~/staging/freetype-2.10.1/src/truetype
directory ~/staging/freetype-2.10.1/src/autofit
directory ~/staging/freetype-2.10.1/src/base
directory ~/staging/freetype-2.10.1/src/bdf
directory ~/staging/freetype-2.10.1/src/bzip2
directory ~/staging/freetype-2.10.1/src/cache
directory ~/staging/freetype-2.10.1/src/cff
directory ~/staging/freetype-2.10.1/src/cid
directory ~/staging/freetype-2.10.1/src/gxvalid
directory ~/staging/freetype-2.10.1/src/gzip
directory ~/staging/freetype-2.10.1/src/lzw
directory ~/staging/freetype-2.10.1/src/otvalid
directory ~/staging/freetype-2.10.1/src/pcf
directory ~/staging/freetype-2.10.1/src/pfr
directory ~/staging/freetype-2.10.1/src/psaux
directory ~/staging/freetype-2.10.1/src/pshinter
directory ~/staging/freetype-2.10.1/src/psnames
directory ~/staging/freetype-2.10.1/src/raster
directory ~/staging/freetype-2.10.1/src/sfnt
directory ~/staging/freetype-2.10.1/src/smooth
directory ~/staging/freetype-2.10.1/src/tools
directory ~/staging/freetype-2.10.1/src/truetype
directory ~/staging/freetype-2.10.1/src/type1
directory ~/staging/freetype-2.10.1/src/type42
directory ~/staging/freetype-2.10.1/src/winfonts
# set trace-commands on
set print inferior-events on

br main



#handle SIGWINCH nostop pass
handle SIGQUIT nostop pass
handle SIGINT stop pass
handle SIGCHLD print stop pass
handle SIGTSTP print nostop pass

define info signal-dispositions
  __isd_print_tbl_hdr

  if $argc == 0
    __isd_info_all_signal_dispositions
  else
    # Looping over args 0..$argc and using eval to extract the current
    # arg doesn't work, because "eval" command misses replacing $argN.
    # See <https://sourceware.org/bugzilla/show_bug.cgi?id=20559>.  We
    # get to unroll the loop manually.
    if $argc >= 1
      __isd_info_signal_disposition $arg0
    end
    if $argc >= 2
      __isd_info_signal_disposition $arg1
    end
    if $argc >= 3
      __isd_info_signal_disposition $arg2
    end
    if $argc >= 4
      __isd_info_signal_disposition $arg3
    end
    if $argc >= 5
      __isd_info_signal_disposition $arg4
    end
    if $argc >= 6
      __isd_info_signal_disposition $arg5
    end
    if $argc >= 7
      __isd_info_signal_disposition $arg6
    end
    if $argc >= 8
      __isd_info_signal_disposition $arg7
    end
    if $argc >= 9
      __isd_info_signal_disposition $arg8
    end
    if $argc >= 10
      __isd_info_signal_disposition $arg9
    end
  end
end

# Helpers go below.

# Print the disposition of a single signal, given by $arg0.
define __isd_info_signal_disposition
  set $_isd_size = sizeof (struct sigaction)
  __isd_mmap $_isd_size
  if $_isd_mmap_res != (void *) -1
    set $_isd_p = (struct sigaction *) $_isd_mmap_res
    __isd_print_disposition $arg0 $_isd_p
    __isd_munmap $_isd_p $_isd_size
  end
end

# Prints the disposition of all signals.
define __isd_info_all_signal_dispositions
  set $_isd_size = sizeof (struct sigaction)
  __isd_mmap $_isd_size
  if $_isd_mmap_res != (void *) -1
    set $_isd_p = (struct sigaction *) $_isd_mmap_res
    set $_isd_i = 1
    set $_isd_nsig = 64
    while $_isd_i < $_isd_nsig
      __isd_print_disposition $_isd_i $_isd_p
      set $_isd_i = $_isd_i + 1
    end
    __isd_munmap $_isd_p $_isd_size
  end
end

# Call mmap in the inferior.  $arg0 is the requested size.  Returns in $_isd_mmap_res
define __isd_mmap
  set $_isd_size = $arg0
  # PROT_READ(1) | PROT_WRITE(2)
  set $_isd_prot = 0x1 | 0x2
  # MAP_PRIVATE(2) | MAP_ANONYMOUS(0x20)
  set $_isd_flags = 0x2 | 0x20
  set $_isd_mmap = (void *(*) (void *, size_t, int, int, int, off_t)) mmap
  set $_isd_mmap_res = $_isd_mmap (0, $_isd_size, $_isd_prot, $_isd_flags, -1, 0)
end

# Call munmap in the inferior.  $arg0 is address, and $arg1 is the size.
define __isd_munmap
  set $_isd_munmap = (int (*) (void *, size_t)) munmap
  call (void) $_isd_munmap ($arg0, $arg1)
end

# Print the table header.
define __isd_print_tbl_hdr
  printf "%s\t%-9s\t%-24s\t%s\n", "Number", "Name", "Description", "Disposition"
  end

# Helper that prints the disposition of a single signal.  First arg is
# signal number, second is sigaction pointer.
define __isd_print_disposition
  set $_isd_sig = $arg0
  set $_isd_p = $arg1
  set $_isd_res = __sigaction ($arg0, 0, $_isd_p)
  if $_isd_res == 0
    printf "%d\t", $_isd_sig
    if $_isd_sig < 32
      __isd_signame $_isd_sig
    else
      printf "SIG%d    ", $_isd_sig
    end
    printf "\t"
    if _new_sys_siglist[$arg0] != 0
      printf "%-24s", _new_sys_siglist[$arg0]
    else
      if $arg0 >= 34
	printf "Real-time signal %d", $arg0 - 34
	printf "%10s", ""
      else
	printf "Unknown signal %d", $arg0
	printf "%10s", ""
      end
    end
    printf "\t"
    # Avoid "__sigaction_handler.sa_handler" because of
    # #define sa_handler __sigaction_handler.sa_handler
    if (long) $_isd_p->__sigaction_handler == 0
      printf "SIG_DFL\n"
    else
      if (long) $_isd_p->__sigaction_handler == 1
	printf "SIG_IGN\n"
      else
	info symbol (long) $_isd_p->__sigaction_handler
      end
    end
  end
end

# Mapping of signal numbers to names.
define __isd_signame
  if $arg0 == 1
    printf "%-9s", "SIGHUP"
  end
  if $arg0 == 2
    printf "%-9s", "SIGINT"
  end
  if $arg0 == 3
    printf "%-9s", "SIGQUIT"
  end
  if $arg0 == 4
    printf "%-9s", "SIGILL"
  end
  if $arg0 == 5
    printf "%-9s", "SIGTRAP"
  end
  if $arg0 == 6
    printf "%-9s", "SIGABRT"
  end
  if $arg0 == 7
    printf "%-9s", "SIGBUS"
  end
  if $arg0 == 8
    printf "%-9s", "SIGFPE"
  end
  if $arg0 == 9
    printf "%-9s", "SIGKILL"
  end
  if $arg0 == 10
    printf "%-9s", "SIGUSR1"
  end
  if $arg0 == 11
    printf "%-9s", "SIGSEGV"
  end
  if $arg0 == 12
    printf "%-9s", "SIGUSR2"
  end
  if $arg0 == 13
    printf "%-9s", "SIGPIPE"
  end
  if $arg0 == 14
    printf "%-9s", "SIGALRM"
  end
  if $arg0 == 15
    printf "%-9s", "SIGTERM"
  end
  if $arg0 == 16
    printf "%-9s", "SIGSTKFLT"
  end
  if $arg0 == 17
    printf "%-9s", "SIGCHLD"
  end
  if $arg0 == 18
    printf "%-9s", "SIGCONT"
  end
  if $arg0 == 19
    printf "%-9s", "SIGSTOP"
  end
  if $arg0 == 20
    printf "%-9s", "SIGTSTP"
  end
  if $arg0 == 21
    printf "%-9s", "SIGTTIN"
  end
  if $arg0 == 22
    printf "%-9s", "SIGTTOU"
  end
  if $arg0 == 23
    printf "%-9s", "SIGURG"
  end
  if $arg0 == 24
    printf "%-9s", "SIGXCPU"
  end
  if $arg0 == 25
    printf "%-9s", "SIGXFSZ"
  end
  if $arg0 == 26
    printf "%-9s", "SIGVTALRM"
  end
  if $arg0 == 27
    printf "%-9s", "SIGPROF"
  end
  if $arg0 == 28
    printf "%-9s", "SIGWINCH"
  end
  if $arg0 == 29
    printf "%-9s", "SIGIO"
  end
  if $arg0 == 30
    printf "%-9s", "SIGPWR"
  end
  if $arg0 == 31
    printf "%-9s", "SIGSYS"
  end
end

#Links:
#  - https://sourceware.org/gdb/onlinedocs/gdb/Forks.html
#  - https://sourceware.org/gdb/onlinedocs/gdb/Inferiors-and-Programs.html
#  - https://stackoverflow.com/questions/39733919/detaching-gdb-without-resuming-the-inferior
#  - https://www.qnx.com/developers/docs/8.0/com.qnx.doc.ide.userguide/topic/debugging_child.html
