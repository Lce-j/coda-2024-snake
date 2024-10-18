NAME = snake

SRCS = snake
SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image
all :  $(NAME)

$(NAME): $(SRCS)
	gcc $(SRCS) -o $(NAME) $(SDL2_CFLAGS) $(SDL2_LDFLAGS)

fclean:
	rm -f $(NAME)

re: fclean all
