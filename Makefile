NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
RM = rm -f

SRC_DIR = srcs

SRCS = $(SRC_DIR)/main.c \
	$(SRC_DIR)/args_parser.c \
	$(SRC_DIR)/setup.c \
	$(SRC_DIR)/setup_utils.c \
	$(SRC_DIR)/thread_manager.c \
	$(SRC_DIR)/thread_manager_join.c \
	$(SRC_DIR)/coder_loop.c \
	$(SRC_DIR)/dongle_grab.c \
	$(SRC_DIR)/scheduler_queue.c \
	$(SRC_DIR)/monitor.c \
	$(SRC_DIR)/clock_log.c \
	$(SRC_DIR)/sim_state.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
