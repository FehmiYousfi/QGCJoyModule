# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++11

# Source and object files
SRC_MAIN = main.cpp
SRC_VJOY = virtual_joystick.cpp
SRC_TEST = TargetVirtual.cpp

OBJ_MAIN = $(SRC_MAIN:.cpp=.o)
OBJ_VJOY = $(SRC_VJOY:.cpp=.o)
OBJ_TEST = $(SRC_TEST:.cpp=.o)

TARGET_MAIN = real_joystick
TARGET_VJOY = virtual_joystick
TARGET_TEST = TargetVirtual

# Libraries (if needed)
LIBS_MAIN = -ludev          # For real_joystick (if required)
LIBS_VJOY =                 # For virtual_joystick (no additional libs)
LIBS_TEST =                 # For TargetVirtual (add libs if needed)

# Default rule
all: $(TARGET_MAIN) $(TARGET_VJOY) $(TARGET_TEST)

# Rule to build real_joystick
$(TARGET_MAIN): $(OBJ_MAIN)
	$(CXX) $(CXXFLAGS) -o $(TARGET_MAIN) $(OBJ_MAIN) $(LIBS_MAIN)

# Rule to build virtual_joystick
$(TARGET_VJOY): $(OBJ_VJOY)
	$(CXX) $(CXXFLAGS) -o $(TARGET_VJOY) $(OBJ_VJOY) $(LIBS_VJOY)

# Rule to build TargetVirtual
$(TARGET_TEST): $(OBJ_TEST)
	$(CXX) $(CXXFLAGS) -o $(TARGET_TEST) $(OBJ_TEST) $(LIBS_TEST)

# Pattern rule to compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove object files and executables
clean:
	rm -f $(OBJ_MAIN) $(OBJ_VJOY) $(OBJ_TEST) $(TARGET_MAIN) $(TARGET_VJOY) $(TARGET_TEST)

# Run the real_joystick program
run_real_joystick: $(TARGET_MAIN)
	sudo ./$(TARGET_MAIN)

# Run the virtual_joystick program
run_virtual_joystick: $(TARGET_VJOY)
	sudo ./$(TARGET_VJOY)

# Run the TargetVirtual program
run_test:
	./$(TARGET_TEST)
