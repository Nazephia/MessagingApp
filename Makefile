# Compilación incremental: cada .cpp genera su propio .o en build/, y -MMD -MP genera un
# archivo .d con las cabeceras que incluye. Así `make` solo recompila los .cpp modificados
# y los que incluyen una cabecera modificada.

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -I.
DEPFLAGS := -MMD -MP

BUILD       := build
TARGET      := $(BUILD)/messaging
TEST_TARGET := $(BUILD)/run_tests

# Todo el código salvo main.cpp y las pruebas. Los archivos nuevos se incluyen solos.
SRC      := $(wildcard models/*.cpp controllers/*.cpp utils/*.cpp)
OBJ      := $(SRC:%.cpp=$(BUILD)/%.o)
MAIN_OBJ := $(BUILD)/main.o
TEST_SRC := $(wildcard tests/*.cpp)
TEST_OBJ := $(TEST_SRC:%.cpp=$(BUILD)/%.o)

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(OBJ) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_TARGET): $(OBJ) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

run: $(TARGET)
	@mkdir -p data
	./$(TARGET)

test: $(TEST_TARGET)
	@mkdir -p $(BUILD)/test_data
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD)

-include $(OBJ:.o=.d) $(MAIN_OBJ:.o=.d) $(TEST_OBJ:.o=.d)
