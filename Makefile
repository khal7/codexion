
NAME = codexion

CC = gcc
CFLAGS = -pthread 
SRCS = src/dongle.c\
		src/heap_utils.c\
		src/main.c\
		src/parsing.c\
		src/request_dongle.c\
		src/routine.c\

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
