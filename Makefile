#	MAKEFLAGS
MAKEFLAGS 				+=		 		--no-print-directory


#	COLORS
ERASE					:=				\033[2K\r
BLUE					:=				\033[34m
YELLOW					:=				\033[33m
GREEN					:=				\033[32m
RED						:=				\033[31m
END						:=				\033[0m


#	EXECUTABLE
NAME					:=				webserv


#	COMPILERS
CXX						:=				c++


#	PROJECT DIRECTORIES
DIR_SRCS				:=				srcs
DIR_INCS				:=				include
DIR_OBJS				:=				.objs
DIR_DEPS				:=				.deps


#	FLAGS
FLAGS					:=				-Wall -Wextra -Werror -I$(DIR_INCS)
DEPFLAGS				:=				-MMD -MP
CXXFLAGS				:=				$(FLAGS) $(DEPFLAGS) -std=c++98


#	SOURCES AND HEADERS
SRCS					:=				$(DIR_SRCS)/main.cpp \
										$(DIR_SRCS)/config/ConfigParser.cpp \
										$(DIR_SRCS)/server/Client.cpp \
										$(DIR_SRCS)/server/EventLoop.cpp \
										$(DIR_SRCS)/server/Server.cpp \
										$(DIR_SRCS)/socket/Socket.cpp \
										$(DIR_SRCS)/socket/SocketClient.cpp \
										$(DIR_SRCS)/socket/SocketServer.cpp \
										$(DIR_SRCS)/http/HttpRequest.cpp \
										$(DIR_SRCS)/http/HttpResponse.cpp \
										$(DIR_SRCS)/http/HttpResponseBuilder.cpp \
										$(DIR_SRCS)/http/Cgi.cpp \
										$(DIR_SRCS)/logger/Logger.cpp \
										$(DIR_SRCS)/logger/ConsoleLogger.cpp \
										$(DIR_SRCS)/logger/FileLogger.cpp \
										$(DIR_SRCS)/utils/Utils.cpp


INCS					:=				$(DIR_INCS)/config/ConfigParser.hpp \
										$(DIR_INCS)/config/Config.hpp \
										$(DIR_INCS)/config/HttpConfig.hpp \
										$(DIR_INCS)/config/LocationConfig.hpp \
										$(DIR_INCS)/config/ServerConfig.hpp \
										$(DIR_INCS)/config/FileLoggerConfig.hpp \
										$(DIR_INCS)/server/Client.hpp \
										$(DIR_INCS)/server/EventLoop.hpp \
										$(DIR_INCS)/server/Server.hpp \
										$(DIR_INCS)/socket/Socket.hpp \
										$(DIR_INCS)/socket/SocketClient.hpp \
										$(DIR_INCS)/socket/SocketServer.hpp \
										$(DIR_INCS)/http/HttpRequest.hpp \
										$(DIR_INCS)/http/HttpResponse.hpp \
										$(DIR_INCS)/http/HttpResponseBuilder.hpp \
										$(DIR_INCS)/http/Cgi.hpp \
										$(DIR_INCS)/logger/Logger.hpp \
										$(DIR_INCS)/logger/ConsoleLogger.hpp \
										$(DIR_INCS)/logger/FileLogger.hpp \
										$(DIR_INCS)/utils/Utils.hpp



#	OBJECTS AND DEPENDENCIES
OBJS					:=				$(addprefix $(DIR_OBJS)/, $(notdir $(SRCS:.cpp=.o)))
DEPS					:=				$(addprefix $(DIR_DEPS)/, $(notdir $(SRCS:.cpp=.d)))


vpath	%.cpp $(DIR_SRCS) $(DIR_SRCS)/server $(DIR_SRCS)/config $(DIR_SRCS)/socket $(DIR_SRCS)/http $(DIR_SRCS)/logger $(DIR_SRCS)/utils


#	ALL RULE
.PHONY:			all
all:
											@if $(MAKE) -q $(NAME); then \
												printf "$(YELLOW)$(NAME) > Nothing to be done$(END)\n"; \
											else \
												$(MAKE) ${NAME}; \
											fi; 


#	COMPILE CPP FILES RULE
$(DIR_OBJS)/%.o:						%.cpp Makefile
											@mkdir -p $(DIR_OBJS) $(DIR_DEPS)
											@$(CXX) $(CXXFLAGS) -c $< -o $@
											@mv $(DIR_OBJS)/$(notdir $(basename $@)).d $(DIR_DEPS)/ 2>/dev/null || true
											@printf "$(BLUE)$(NAME) > Compiling : $(END)$<\n"


#	LINKING
${NAME}:								${OBJS}
											@$(CXX) $(CXXFLAGS) $(OBJS) -o $@
											@printf "$(GREEN)$(NAME) > Done Compiling : $(END)$@\n"


#	INCLUDE DEPENDENCIES FILES
-include	$(DEPS)


#	CLEAN RULE
.PHONY:			clean
clean:
											@if [ ! -d $(DIR_OBJS) ] && [ ! -d $(DIR_DEPS) ]; then \
												printf "$(RED)$(NAME) > Nothing to clean $(END)\n"; \
											else \
												[ -d $(DIR_OBJS) ] && \
													rm -rf $(DIR_OBJS) && \
													printf "$(RED)$(NAME) > Done deleting : $(END)$(DIR_OBJS)\n"; \
												[ -d $(DIR_DEPS) ] && \
													rm -rf $(DIR_DEPS) && \
													printf "$(RED)$(NAME) > Done deleting : $(END)$(DIR_DEPS)\n"; \
											fi; true

#	FCLEAN RULE
.PHONY:			fclean
fclean:			clean
											@if [ -f $(NAME) ]; then \
												rm -rdf $(NAME); \
												printf "$(RED)$(NAME) > Done deleting : $(END)$(NAME)\n"; \
											fi;


#	REBUILD RULE
.PHONY:			re
re:				fclean	all
				

#	DEBUG RULE
.PHONY:			debug
debug:
										$(MAKE) re FLAGS="$(FLAGS) -g3"


#	RUN RULE
.PHONY:			run
run:									$(MAKE) all FLAGS="$(FLAGS) -O3"
										./$(NAME)
