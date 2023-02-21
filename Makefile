CC=g++ -g -Wall -fno-builtin -std=c++17

# List of source files for your pager
PAGER_SOURCES=vm_pager.cpp page_structure.cpp

# Generate the names of the pager's object files
PAGER_OBJS=${PAGER_SOURCES:.cpp=.o}

all: pager app test1 test2 test3 test4 test5 test6 test7 test8 test9 test10 test11 test12 test13 test14 test15

# Compile the pager and tag this compilation
pager: ${PAGER_OBJS} libvm_pager.o
	./autotag.sh
	${CC} -o $@ $^

# Compile an application program
app: app.cpp libvm_app.o
	${CC} -o $@ $^ -ldl
test%: test_%.4.cpp libvm_app.o
	${CC} -o $@ $^ -ldl
test%: test_%.5.cpp libvm_app.o
	${CC} -o $@ $^ -ldl
# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${PAGER_OBJS} pager app test1 test2 test3 test4 test5 test6 test7 test8 test9 test10 test11 test12 test13 test14 test15
