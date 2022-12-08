CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic 

ifdef SANITIZE
	CFLAGS = -Wall -Wextra -Werror -g3 -fsanitize=address
endif

NAME = webserv

INC_DIR = ../include/

SRC_DIR = ../src/

INC_FLAG = -I$(INC_DIR)

SRC_FILES = $(addprefix $(SRC_DIR),\
							main.cpp\
      				RequestParser.cpp\
      				ServerUtil.cpp\
      				ServerManager.cpp\
      				Server.cpp\
      				Parser.cpp\
      				HTTPResponse.cpp\
      				RequestProcessor.cpp\
      				Location.cpp\
      				ThreadPool.cpp\
      				CGI.cpp\
      				Session.cpp)

OBJ = ${SRC_FILES:.cpp=.o}

all 	: $(NAME)

$(NAME)	: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(INC_FLAG)

%.o 	: %.cpp
	$(CC) $(CFLAGS) $(INC_FLAG) -c $< -o $@

clean	:
	rm -f $(OBJ)

fclean	:
	rm -f $(NAME)
	$(MAKE) clean

re		:
	$(MAKE) fclean
	$(MAKE) all

.PHONY	: clean fclean re all