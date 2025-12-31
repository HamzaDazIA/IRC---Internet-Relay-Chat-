

SRC = Channel/Channel.cpp Client/Client.cpp Server/Server.cpp
NAME = server
OBJ = $(SRC:.c=.o)
CC = cc
CFLAGS = -Wall -Wextra  -Werror
RM = rm -f

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: clean 