NAME						:=	\
	ircserv

SRC_DIRS		 			:= \
	src

BUILD_DIR					:= \
	build

SRCS						:= \
	$(shell find $(SRC_DIRS) -name '*.cpp')

OBJS					:= \
	$(SRCS:%=$(BUILD_DIR)/%.o)

I							:= \
	$(addprefix -I, $(shell find include -type d))

DEPS						:= \
	$(OBJS:.o=.d)

C							:= clang++
CPPFLAGS					:= -g -Wall -Wextra -Werror -std=c++98 
DEPFLAGS					:= -MMD -MP

R							:= rm -f

$(BUILD_DIR)/%.cpp.o:		%.cpp
	mkdir -p $(dir $@)
	$C $(CPPFLAGS) $(DEPFLAGS) $I -c $< -o $@

all:						$(NAME)

debug: CFLAGS += -DDEBUG -g
debug: all

$(NAME):					$(OBJS)
	$C $^ -o $@

clean:
	$R $(OBJS)

fclean:						clean
	$R $(NAME) $(DEPS)

re:							fclean all

.PHONY: 					all clean fclean re

-include $(DEPS)
