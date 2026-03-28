# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: prasingh <prasingh@student.42berlin.de>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/05 00:00:00 by prasingh          #+#    #+#              #
#    Updated: 2025/02/05 00:00:00 by prasingh         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME		= codexion

SRC_DIR		= coders
OBJ_DIR		= obj

SRCS		= main.c main_parse.c main_run.c utils.c parse_args.c \
			  dongle_time.c coder.c coder_dongle.c dongle_pair.c sim_init.c \
			  sim_alloc.c sim_cleanup.c sim_dongle.c monitor.c \
			  priority_queue.c pq_heap.c geometry.c

OBJS		= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
INCLUDE		= -I$(SRC_DIR)

all: $(OBJ_DIR) $(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/codexion.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
