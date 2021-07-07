CXX = gcc
OBJECTS = main.o avl_tree.o
RM = rm -f
OUT = a.out
CLIP_SIZE = 20
BASE_DIR = "/home/emil/Documents/Programming/C_Files/Clipboard"
CURRENT_CLIP_FILE = "/home/emil/Documents/Programming/C_Files/Clipboard/Resources/current_clipboard.txt"
HISTORY_CLIP_FILE = "/home/emil/Documents/Programming/C_Files/Clipboard/Resources/clipboard_history.txt"
CLIP_READ_SCRIPT = "/home/emil/Documents/Programming/C_Files/Clipboard/src/_run_parsellite_.sh"
DELAY = 10 # delay in seconds
DAEMON_PID = "/home/emil/Documents/Programming/C_Files/Clipboard/Resources/daemon_pid.txt"
LOG_FILE = "/home/emil/Documents/Programming/C_Files/Clipboard/Resources/log.txt"
STDOUT = "/home/emil/Documents/Programming/C_Files/Clipboard/Resources/stdout.txt"
STDERR = "/home/emil/Documents/Programming/C_Files/Clipboard/Resources/stderr.txt"
GIT_SYNCH = "/home/emil/Documents/Programming/C_Files/Clipboard/src/_git_synch_.sh"
GIT_CLONE = "/home/emil/Documents/Programming/C_Files/Clipboard/src/_git_clone_.sh"

all: compile

main.o: ./src/main.c ./avl_tree.o
	$(CXX) -c ./src/main.c

avl_tree.o: ./src/avl_tree.h ./src/avl_tree.c
	$(CXX) -c ./src/avl_tree.c

compile: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(OUT)

run: $(OBJECTS)
	./$(OUT) $(CLIP_SIZE) $(CURRENT_CLIP_FILE) $(HISTORY_CLIP_FILE) $(CLIP_READ_SCRIPT) $(DELAY) $(DAEMON_PID) $(LOG_FILE) $(STDOUT) $(STDERR) $(GIT_SYNCH) $(BASE_DIR) $(GIT_CLONE)

clean:
	$(RM) *.o
	@if [ -e ${OUT} ]; then rm -f ${OUT}; fi