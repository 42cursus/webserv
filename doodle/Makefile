# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abelov <abelov@student.42london.com>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/01/03 23:34:27 by abelov            #+#    #+#              #
#    Updated: 2026/01/04 00:04:06 by abelov           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			:= doodle

BUILD_DIR		= build
INC_DIR			= ./include
SRC_DIR			= src

CC				:= gcc

INCLUDE_FLAGS	:= -I. -I$(INC_DIR)
DEBUG_FLAGS		:= -g3 -gdwarf-3 -Wa,-L
MANDATORY_FLAGS	:= -Wall -Wextra -Werror -Wimplicit -Wstrict-aliasing=2
DIAG_FLAGS		:=
LINK_FLAGS		:= -lm -O3 -march=native -maes \
					-Wl,-O3,-Bsymbolic-functions,--as-needed
COMMON_FLAGS	= -mmanual-endbr \
					-falign-functions=32 \
					-fno-semantic-interposition \
					-fasynchronous-unwind-tables \
					-fno-stack-clash-protection \
					-fcf-protection=none \
					-fno-stack-protector \
					-fno-omit-frame-pointer  \
#					-fstrict-aliasing \
					-fno-strict-overflow
OPTIMIZE_FLAGS	:= -O0 -march=native -mtune=native $(COMMON_FLAGS)
COMMON_CFLAGS	= $(MANDATORY_FLAGS) $(DEBUG_FLAGS) $(OPTIMIZE_FLAGS) \
					$(INCLUDE_FLAGS) $(DIAG_FLAGS) -DCGI_DIR="\"/cgi-bin/\""

RMFLAGS			= -r

_SRCS			= main.c \
					process_request.c \
					setup_server.c
SRCS			:= $(_SRCS:%=$(SRC_DIR)/%)
OBJS			= $(SRCS:%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: %.c
		@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
		$(CC) $(COMMON_CFLAGS) -c -o $@ $<

all: $(NAME)

$(NAME): $(OBJS)
		$(CC) $(COMMON_CFLAGS) $(LINK_FLAGS) $^ -o $@

## clean
clean:
		@if [ -d $(BUILD_DIR) ]; then $(RM) $(RMFLAGS) $(BUILD_DIR); fi

## fclean
fclean: clean
		@if [ -f $(NAME) ]; then $(RM) $(RMFLAGS) $(NAME); fi

## re
re: fclean
		+$(MAKE) all

.PHONY: all clean fclean re
