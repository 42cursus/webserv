# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: margo <margo@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/17 22:30:53 by abelov            #+#    #+#              #
#    Updated: 2025/08/20 20:21:46 by fsmyth           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME 			= webserv

BUILD_DIR		= build
INC_DIR			= ./include

CFLAGS 			:= -Wall -Wextra -Werror -std=c++98 -g3 -gdwarf-3 -O0
CPP 			= c++

INCLUDE_FLAGS	:= -I. -I$(INC_DIR) -I/usr/include


SRCS			= src/main.cpp \
				  src/app/Worker.cpp \
				  src/app/WorkerPool.cpp \
				  src/app/TCPServer.cpp \
				  src/http/HttpRequest.cpp \
				  src/http/HttpResponse.cpp \
				  src/utils/Parser.cpp \
				  src/utils/State.cpp \
				  src/utils/Utils.cpp


OBJS			= $(SRCS:%.cpp=$(BUILD_DIR)/%.o)


ifeq ($(MAKELEVEL),0)
	# Only set --jobs if user didn't already pass a -j option manually
	ifeq ($(filter -j,$(MAKEFLAGS)),)
		MAKEFLAGS += --jobs=$(shell nproc) --no-print-directory #--quiet
	endif
endif

all: $(NAME)

$(NAME): $(OBJS)
		$(CPP) $(CFLAGS) $(OBJS) -o $@
		@echo "$(NAME) BUILD COMPLETE!"

$(BUILD_DIR)/%.o: %.cpp
		@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
		$(CPP) $(CFLAGS) $(INCLUDE_FLAGS) -c $^ -o $@

## clean
clean:
		@$(RM) -rf $(OBJS)

## fclean
fclean: clean
		@$(RM) -vf $(NAME)

re: fclean
		+$(MAKE) all

.PHONY: all clean fclean re
