CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS = -lrt -pthread

TARGET = main
SOURCE = main.cpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o

help:
	@echo "Available targets:"
	@echo "  all   - Build the program"
	@echo "  run   - Build and run the program"
	@echo "  clean - Remove build artifacts"
