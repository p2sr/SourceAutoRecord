BINARY=sar.so
STEAM=/home/nekz/.local/share/Steam/steamapps/common/
SDIR=src/
LDIR=lib/
ODIR=obj/

SRCS=$(wildcard $(SDIR)*.cpp)
SRCS+=$(wildcard $(SDIR)Features/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/Demo/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/Hud/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/Speedrun/*.cpp)
SRCS+=$(wildcard $(SDIR)Games/Linux/*.cpp)
SRCS+=$(wildcard $(SDIR)Modules/*.cpp)
OBJS=$(patsubst $(SDIR)%.cpp, $(ODIR)%.o, $(SRCS))

CC=g++
STFU=-Wno-unused-function -Wno-unused-variable -Wno-parentheses
CFLAGS=-std=c++17 -m32 -fPIC -static-libstdc++ -shared -Wall $(STFU) -I$(LDIR) -I$(SDIR)
EXPORT=cp -fu

sar: $(OBJS)
	$(CC) $(CFLAGS) -o $(BINARY) $^ -lstdc++fs
	@$(EXPORT) "$(BINARY)" "$(STEAM)Portal 2/$(BINARY)"
	@$(EXPORT) "$(BINARY)" "$(STEAM)The Stanley Parable/$(BINARY)"
	@$(EXPORT) "$(BINARY)" "$(STEAM)The Beginners Guide/$(BINARY)"
	@$(EXPORT) "$(BINARY)" "$(STEAM)Half-Life 2/$(BINARY)"
	@$(EXPORT) "$(BINARY)" "$(STEAM)Portal/$(BINARY)"

$(ODIR)%.o: $(SDIR)%.cpp $(SDIR)%.hpp
	@echo $@
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -rf $(ODIR)SAR.o $(BINARY)

clean-all:
	@rm -rf $(OBJS) $(BINARY)