#CXX=mips-g++
CXX=g++

#SRC += test.c
#OBJ += $(SRC:.c=.o)

######以上两个可以改成这样，方便扩展#######################################
#列举当前当前目录所有的.c文件
#SRC += $(wildcard *.c) 
#把SRC中的.c替换成.o
#OBJ += $(patsubst %.c, %.o, $(SRC))
SRC += $(wildcard *.cpp) 
#把SRC中的.c替换成.o
OBJ += $(patsubst %.cpp, %.o, $(SRC))
###########################################################################

#################################LD INCLUDE################################
LDFLAG += -L/usr/local/lib  -lfdk-aac
IFLAG += -I /usr/local/include/fdk-aac
#LDFLAG = -Xlinker "-(" -lfdk-aac -Xlinker "-)" -L ./lib

###########################################################################
TARGET = test
all:$(TARGET)
$(TARGET):$(OBJ)
	$(CXX) -o $@ $^ $(LDFLAG) $(IFLAG)
			
.cpp.o:
	$(CXX) -c $< -o $@ $(LDFLAG) $(IFLAG)
.PHTHON:clean
clean:
	rm -rf $(TARGET) $(OBJ)


