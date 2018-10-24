TARGET:=ALEXA_WAKEUP
TEST:=alexa_test
PER_PATH:=$(shell pwd)
CC:=g++

CCFLAG:= -DFOR_C_APP -o0 -Wall -g -std=c++0x
LDFLAG:= -o 

.PHONY : all clean

all: precompile $(TARGET)

precompile:
	@echo ""
	@echo "--- Building: $(TARGET) ---"
	 @echo $(PER_PATH)
	
$(TARGET):
	@$(CC) $(CCFLAG) main.cpp  ../src/SensoryWakeWordEngine.cpp ../src/Logger.cpp ../src/WakeWordException.cpp $(LDFLAG) $(TEST)  -I ../src -L../lib -lsnsr
	@echo "compile is ok"
clean:
	@echo "clean $(TEST)"
	@rm  -rf *.o
	@rm -rf  $(TEST) 
