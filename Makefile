NAME		= hsrtcsim
SRCS		= main.cpp
OBJS		= $(SRCS:.cpp=.o)
CXX			= g++
CXXFLAGS	= -Wall -std=c++20
INCDIR		= -I/usr/include/hredis
LIBDIR		= -L/usr/lib
LDFLAGS		= -lhiredis -levent -lstdc++

.PHONY: all clean fclean re
all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS) $(INCDIR) $(LIBDIR) $(LDFLAGS)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all