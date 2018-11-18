BINARY=sar.so
STEAM=/home/nekz/.steam/steamapps/common/
SDIR=src/
LDIR=lib/
ODIR=obj/

SRCS=$(wildcard $(SDIR)*.cpp)
SRCS+=$(wildcard $(SDIR)Features/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/Demo/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/Hud/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/Routing/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/Speedrun/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/Speedrun/Rules/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/Stats/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/Tas/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/Timer/*.cpp)
SRCS+=$(wildcard $(SDIR)Games/Linux/*.cpp)
SRCS+=$(wildcard $(SDIR)Modules/*.cpp)
SRCS+=$(wildcard $(SDIR)Utils/*.cpp)
OBJS=$(patsubst $(SDIR)%.cpp, $(ODIR)%.o, $(SRCS))

CC=g++
STFU=-Wno-unused-function -Wno-unused-variable -Wno-parentheses -Wno-unknown-pragmas
CFLAGS=-std=c++17 -m32 -fPIC -static-libstdc++ -shared -Wall $(STFU) -I$(LDIR) -I$(SDIR)
EXPORT=cp -fu

all: dirs | sar

sar: $(OBJS)
	@$(CC) $(CFLAGS) -o $(BINARY) $^ -lstdc++fs
	@$(EXPORT) "$(BINARY)" "$(STEAM)Portal 2/$(BINARY)"
	@$(EXPORT) "$(BINARY)" "$(STEAM)Aperture Tag/$(BINARY)"
	@$(EXPORT) "$(BINARY)" "$(STEAM)Portal Stories Mel/$(BINARY)"
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

dirs:
	@mkdir -p $(ODIR)
	@mkdir -p $(ODIR)Features/
	@mkdir -p $(ODIR)Features/Demo/
	@mkdir -p $(ODIR)Features/Hud/
	@mkdir -p $(ODIR)Features/Routing/
	@mkdir -p $(ODIR)Features/Speedrun/
	@mkdir -p $(ODIR)Features/Speedrun/Rules
	@mkdir -p $(ODIR)Features/Stats/
	@mkdir -p $(ODIR)Features/Tas/
	@mkdir -p $(ODIR)Features/Timer/
	@mkdir -p $(ODIR)Games/Linux/
	@mkdir -p $(ODIR)Modules/
	@mkdir -p $(ODIR)Utils/
