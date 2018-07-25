BINARY=sar.so
STEAM=/home/nekz/.local/share/Steam/steamapps/common/
SDIR=src/SourceAutoRecord/
LDIR=libs/
ODIR=obj/

SRCS=$(wildcard $(SDIR)*.cpp)
SRCS+=$(wildcard $(SDIR)Modules/*.cpp)
SRCS+=$(wildcard $(SDIR)Features/*.cpp)
OBJS=$(patsubst $(SDIR)%.cpp, $(ODIR)%.o, $(SRCS))

CC=g++
CFLAGS=-std=c++14 -m32 -fPIC -static-libstdc++ -shared -Wall -Wno-unused-function -I$(LDIR) -I$(SDIR)
EXPORT=cp -fu

sar: $(OBJS)
	$(CC) $(CFLAGS) -o $(BINARY) $^
	@$(EXPORT) "$(BINARY)" "$(STEAM)Portal 2/$(BINARY)"
	@#$(EXPORT) "$(BINARY)" "$(STEAM)The Stanley Parable/$(BINARY)"
	@#$(EXPORT) "$(BINARY)" "$(STEAM)The Beginners Guide/$(BINARY)"
	@#$(EXPORT) "$(BINARY)" "$(STEAM)Half-Life 2/$(BINARY)"
	@#$(EXPORT) "$(BINARY)" "$(STEAM)Portal/$(BINARY)"

$(ODIR)%.o: $(SDIR)%.cpp $(SDIR)%.hpp
	@echo $@
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -rf $(OBJS) $(BINARY)