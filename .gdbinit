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

echo loading .gdbinit from: ${PROJECT_ROOT} \n
set verbose on

#layout reg
#set disassemble-next-line on
#set print static-members off
#set print pretty on
#set debug auto-load on
set height unlimited
set pagination off
set confirm off
set max-value-size unlimited

set follow-fork-mode parent
set detach-on-fork on

set breakpoint pending on
set print inferior-events on

source ./.runtime.gdbinit

#Links:
#  - https://sourceware.org/gdb/onlinedocs/gdb/Forks.html
#  - https://sourceware.org/gdb/onlinedocs/gdb/Inferiors-and-Programs.html
#  - https://stackoverflow.com/questions/39733919/detaching-gdb-without-resuming-the-inferior
#  - https://www.qnx.com/developers/docs/8.0/com.qnx.doc.ide.userguide/topic/debugging_child.html
