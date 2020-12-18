#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include "avl_tree.h"

// clipboard item structure
typedef struct _item {
    char * elem;
    struct _item * prev, * next;
} Item;

// convert string to integer
int str_to_int(char *);
// reverse string
void str_reverse(char *, int);
// convert integer to string
char * int_to_str(int);
// compare function for avl_tree
int my_compare(const void *, const void *);
// custom print function for AVL tree nodes
void * my_print(void *);
// create Item structure element
Item * create_item(char *);
// helper to insert the node with the key
int insert_item(Item **, Item **, char *, int *, int, int);
// helper to delete the node with the key in it
void delete_item(Item **, Item **, char *, int *);
// find item in the queue
Item * find_item(Item *, char *);
// simply iterate through list and print items
void iterate_n_print(Item *);
// parse tmp.txt file
char * parse_file(char *);
// write to history file
void write_to_file(Item *, char *);
// write to log file
void log_file_write(char *, char *);
// free queue and arguments
void free_queue(Item *, char **, int);
// SIGINT handler
void sig_handler(int);
// run another program
void run_exec(char **, char **, Item *, char *, int, int, int);
// copy one string to another
char * str_copy(char *);
// free 2D array
void free_2d_array(char **, int);
// read clipboard history from file
void read_clip_history(Item **, Item **, char *, int *, int);
// append one string to another
void str_append(char **, char *, int *);
// free only the queue, without freeing args
void free_only_queue(Item *);


int main(int argc, char ** argv) {
    /* queue related variables */
    Item * items_start = NULL, * items_end = NULL; // start and end of the items queue
    int current_queue_size = 0;
    int size_of_clipboard;
    /* daemon related variables */
    char ** args_for_exec; // exec arguments to run another program
    char * clip_contents;
    time_t exec_time;
    FILE * fp_pid, * fp_log;
    char * str_daemon_pid;
    int flag_inserted; // dirty bit to check if tree was changed
    char * parent_pid; // pid of the parent of the process that will run execl()
    int fd_out, fd_err; // file descriptors for STDOUT and STDERR
    int args_exec_size; // size of arguments sent to exec
    /* argument variables */
    int args_size = 10;
    char * CURRENT_CLIP_FILE; // file to write the current clipboard contents
    char * HISTORY_CLIP_FILE; // file where the history of the clipboard will be written to
    // name of the shell script that will read-
    // the current clipboard contents and write it into CURRENT_CLIP_FILE file
    char * CLIP_READ_SCRIPT;
    int DELAY; // delay in seconds
    char * DAEMON_PID; // file where the daemon's PID will be stored
    char * LOG_FILE; // log file
    char * STDOUT; // file, where all stdout messages will be redirected
    char * STDERR; // file, where all stderr messages will be redirected
    char * GIT_SYNCH; // git synchronization script
    char * BASE_DIR; // base dir of the project
    char * GIT_CLONE; // git cloning script
    char ** args_to_free; // string arguments that should be freed

    /* parse arguments */
    if(argc < 13) { // not enough arguments
        printf("Not enough arguments: only %d out of %d are present\n", argc - 1, 12);
        return 0;
    }
    size_of_clipboard = str_to_int(argv[1]);
    // if customly selected clipboard size < 1, change size back to 1
    if(size_of_clipboard < 1)
        size_of_clipboard = 1;
    else if(size_of_clipboard > 25)
        size_of_clipboard = 25;
    CURRENT_CLIP_FILE = (char *) malloc((strlen(argv[2]) + 1) * sizeof(char));
    strncpy(CURRENT_CLIP_FILE, argv[2], strlen(argv[2]));
    CURRENT_CLIP_FILE[strlen(argv[2])] = '\0';
    HISTORY_CLIP_FILE = (char *) malloc((strlen(argv[3]) + 1) * sizeof(char));
    strncpy(HISTORY_CLIP_FILE, argv[3], strlen(argv[3]));
    HISTORY_CLIP_FILE[strlen(argv[3])] = '\0';
    CLIP_READ_SCRIPT = (char *) malloc((strlen(argv[4]) + 1) * sizeof(char));
    strncpy(CLIP_READ_SCRIPT, argv[4], strlen(argv[4]));
    CLIP_READ_SCRIPT[strlen(argv[4])] = '\0';
    DELAY = str_to_int(argv[5]);
    DAEMON_PID = (char *) malloc((strlen(argv[6]) + 1) * sizeof(char));
    strncpy(DAEMON_PID, argv[6], strlen(argv[6]));
    DAEMON_PID[strlen(argv[6])] = '\0';
    LOG_FILE = (char *) malloc((strlen(argv[7]) + 1) * sizeof(char));
    strncpy(LOG_FILE, argv[7], strlen(argv[7]));
    LOG_FILE[strlen(argv[7])] = '\0';
    STDOUT = (char *) malloc((strlen(argv[8]) + 1) * sizeof(char));
    strncpy(STDOUT, argv[8], strlen(argv[8]));
    STDOUT[strlen(argv[8])] = '\0';
    STDERR = (char *) malloc((strlen(argv[9]) + 1) * sizeof(char));
    strncpy(STDERR, argv[9], strlen(argv[9]));
    STDERR[strlen(argv[9])] = '\0';
    GIT_SYNCH = (char *) malloc((strlen(argv[10]) + 1) * sizeof(char));
    strncpy(GIT_SYNCH, argv[10], strlen(argv[10]));
    GIT_SYNCH[strlen(argv[10])] = '\0';
    BASE_DIR = (char *) malloc((strlen(argv[11]) + 1) * sizeof(char));
    strncpy(BASE_DIR, argv[11], strlen(argv[11]));
    BASE_DIR[strlen(argv[11])] = '\0';
    GIT_CLONE = (char *) malloc((strlen(argv[12]) + 1) * sizeof(char));
    strncpy(GIT_CLONE, argv[12], strlen(argv[12]));
    GIT_CLONE[strlen(argv[12])] = '\0';
    // initialize args_to_free array
    args_to_free = (char **) malloc(args_size * sizeof(char *));
    args_to_free[0] = CURRENT_CLIP_FILE;
    args_to_free[1] = HISTORY_CLIP_FILE;
    args_to_free[2] = CLIP_READ_SCRIPT;
    args_to_free[3] = DAEMON_PID;
    args_to_free[4] = LOG_FILE;
    args_to_free[5] = STDOUT;
    args_to_free[6] = STDERR;
    args_to_free[7] = GIT_SYNCH;
    args_to_free[8] = BASE_DIR;
    args_to_free[9] = GIT_CLONE;
    
    /* initialize an AVL tree(set custom compare function to it) */
    //assign_compare_func(&my_compare);

    /* create our daemon */
    pid_t pid;
    // unmask the file, so that it has all the permissions
    umask(0);
    // first fork: it is done to create a new process with different process ID,-
    // but in the same process group(prerequisite before setsid())
    pid = fork();
    if(pid != 0) exit(0); // parent process
    // create a new session
    // this newly created child will create a new session, set itself as its leader-
    // will also create a new process group and set itself as its leader-
    // thus, it will not have a controlling terminal
    setsid();
    // second fork: it is done to prevent second child from being session leader-
    // so, that it can't assign itself a controlling terminal
    pid = fork();
    if(pid != 0) exit(0); // parent process
    // change directory to root
    //chdir("/");

    /* open the log file */
    if(!(fp_log = fopen(LOG_FILE, "w"))) {
        printf("Couldn't open log file: %s\n", LOG_FILE);
        // free all allocated resources
        free_queue(items_start, args_to_free, args_size);
        exit(1);
    }
    fwrite("__START__\n", 1, strlen("__START__\n"), fp_log);
    fclose(fp_log);

    // close all open file descriptors(except log file descriptor)
    // as I only have std{in,out} and stderr as open file descriptors, so I will close them
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    // now we are actually running as a daemon :)

    // write the daemon PID to the daemon_pid.txt file, to kill in the future(if needed)
    if(!(fp_pid = fopen(DAEMON_PID, "w"))) {
        log_file_write("Couldn't open DAEMON_PID file.", LOG_FILE);
        // free all allocated resources
        free_queue(items_start, args_to_free, args_size);
        exit(1);
    }
    str_daemon_pid = int_to_str(getpid());
    fwrite(str_daemon_pid, 1, strlen(str_daemon_pid), fp_pid);
    free(str_daemon_pid);
    fclose(fp_pid);

    /* open stdout.txt and stderr.txt */
    fd_out = open(STDOUT, O_RDWR | O_CREAT | O_TRUNC);
    fd_err = open(STDERR, O_RDWR | O_CREAT | O_TRUNC);
    dup2(fd_out, 1);
    dup2(fd_err, 2);
    close(fd_out);
    close(fd_err);

    /* running the loop that will read from clipboard and edit file */
    // the clipboard history will be written to the HISTORY_CLIP_FILE file-
    // every DELAY minute-ish
    signal(SIGINT, sig_handler); // terminate on SIGINT
    exec_time = time(NULL);
    parent_pid = int_to_str(getpid());
    flag_inserted = 0; // dirty bit to check if tree was changed
    while(1) {
        // pull from git repo
        args_for_exec = (char **) malloc(4 * sizeof(char *));
        args_for_exec[0] = str_copy(GIT_CLONE);
        args_for_exec[1] = str_copy(BASE_DIR);
        args_for_exec[2] = str_copy(parent_pid);
        args_for_exec[3] = (char *) NULL;
        args_exec_size = 4;
        run_exec(args_to_free, args_for_exec, items_start, LOG_FILE, args_size, args_exec_size, 0);
        free_2d_array(args_for_exec, args_exec_size);
        // synch history file with linked list
        read_clip_history(&items_start, &items_end, HISTORY_CLIP_FILE, &current_queue_size, size_of_clipboard);

        // run exec to read current clipboard(execl(CLIP_READ_SCRIPT, CLIP_READ_SCRIPT, CURRENT_CLIP_FILE, parent_pid, (char *) NULL))
        args_for_exec = (char **) malloc(4 * sizeof(char *));
        args_for_exec[0] = str_copy(CLIP_READ_SCRIPT);
        args_for_exec[1] = str_copy(CURRENT_CLIP_FILE);
        args_for_exec[2] = str_copy(parent_pid);
        args_for_exec[3] = (char *) NULL;
        args_exec_size = 4;
        run_exec(args_to_free, args_for_exec, items_start, LOG_FILE, args_size, args_exec_size, 5);
        free_2d_array(args_for_exec, args_exec_size);
        // read the current contents of the clipboard and write them into AVL tree
        clip_contents = parse_file(CURRENT_CLIP_FILE);
        if(insert_item(&items_start, &items_end, clip_contents, &current_queue_size, size_of_clipboard, 0))
            flag_inserted = 1;
        free(clip_contents);

        /* write into the history file */
        // write to the history file, only if clipboard was updated
        // if DELAY minutes have passed since the previous write, write this AVL tree values into file
        if(flag_inserted && (long) time(NULL) - (long) exec_time > (long) DELAY) {
            write_to_file(items_start, HISTORY_CLIP_FILE);
            
            /* synch with git repo */
            // run exec to synch with git repo(execl(GIT_SYNCH, GIT_SYNCH, BASE_DIR, parent_pid, (char *) NULL))
            args_for_exec = (char **) malloc(4 * sizeof(char *));
            args_for_exec[0] = str_copy(GIT_SYNCH);
            args_for_exec[1] = str_copy(BASE_DIR);
            args_for_exec[2] = str_copy(parent_pid);
            args_for_exec[3] = (char *) NULL;
            args_exec_size = 4;
            run_exec(args_to_free, args_for_exec, items_start, LOG_FILE, args_size, args_exec_size, 0);
            free_2d_array(args_for_exec, args_exec_size);
            
            /* maintenance stuff */
            exec_time = time(NULL); // reset execution time
            flag_inserted = 0; // reset dirty bit
        }
    }

    /* write SUCCESS into the log file and close it */
    log_file_write("__SUCCESS__", LOG_FILE);

    /* free all allocated resources */
    free_queue(items_start, args_to_free, args_size);
    free(parent_pid);
    
    return 0;
}

// daemon related
char * parse_file(char * CURRENT_CLIP_FILE) {
    char * str = NULL, c;
    int count = 0, len = 1;
    FILE * fp = fopen(CURRENT_CLIP_FILE, "r");
    // read the file
    c = fgetc(fp); // first character is '\n', so skip it
    if(c == EOF) return NULL;
    while((c = (char) fgetc(fp)) && c != EOF) {
        if(count == len - 1)
            str = (char *) realloc(str, (len *= 2) * sizeof(char));
        str[count] = c;
        count++;
    }
    str[count] = '\0';
    return str;
}

void write_to_file(Item * items_start, char * HISTORY_CLIP_FILE) {
    Item * tmp = items_start;
    FILE * fp = fopen(HISTORY_CLIP_FILE, "w");
    char str[5], prev = '\0';
    int count = 0;
    while(tmp) {
        str[0] = (char) (count / 10 + 48);
        str[1] = (char) (count % 10 + 48);
        str[2] = ':';
        str[3] = '\n';
        str[4] = '\0';
        fwrite(str, 1, strlen(str), fp);
        for(int i = 0; i < strlen(tmp->elem); i++) {
            if((i == 0 || prev == '\n') && i <= strlen(tmp->elem) - 3 && isdigit((tmp->elem)[i]) &&
                isdigit((tmp->elem)[i + 1]) && (tmp->elem)[i + 2] == ':')
            {
                fputc('\\', fp);
            }
            fputc((tmp->elem)[i], fp);
            prev = (tmp->elem)[i];
        }
        fputc('\n', fp);
        tmp = tmp->next;
        count++;
    }
    fclose(fp);
}

void log_file_write(char * argv, char * LOG_FILE) {
    FILE * fp_log = fopen(LOG_FILE, "a");
    fwrite(argv, 1, strlen(argv), fp_log);
    fputc('\n', fp_log);
    fclose(fp_log);
}

void sig_handler(int para_sig) {
    exit(1);
}

void run_exec(char ** args_to_free, char ** args_for_exec, Item * items_start, char * LOG_FILE, int args_size, int args_exec_size, int timeout) {
    int pid, signal_received;
    sigset_t signals_set;
    sigemptyset(&signals_set); // reset the signals set
    sigaddset(&signals_set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &signals_set, NULL);
    // run the clipboard reading script
    pid = fork();
    if(pid < 0) { // error when forking
        log_file_write("Forking error: before first exec().", LOG_FILE);
        // free all allocated resources
        free_queue(items_start, args_to_free, args_size);
        free_2d_array(args_for_exec, args_exec_size);
        exit(1);
    }
    if(!pid) { // child process
        // run clipboard reading script
        sleep(timeout); // sleep, so that parcellite doesn't return "core dumped" error
        //execl(CLIP_READ_SCRIPT, CLIP_READ_SCRIPT, CURRENT_CLIP_FILE, parent_pid, (char *) NULL);
        execv(args_for_exec[0], args_for_exec);
        exit(1); // if execl() could not run
    }
    // wait until SIGUSR1 is sent from execl() program
    if(sigwait(&signals_set, &signal_received)) {
        log_file_write("Error: sigwait().", LOG_FILE);
        free_queue(items_start, args_to_free, args_size);
        free_2d_array(args_for_exec, args_exec_size);
        exit(1);
    }
    if(signal_received != SIGUSR1) {
        char * str = int_to_str(signal_received);
        log_file_write("Incorrect signal:", LOG_FILE);
        log_file_write(str, LOG_FILE);
        free(str);
        free_queue(items_start, args_to_free, args_size);
        free_2d_array(args_for_exec, args_exec_size);
        exit(1);
    }
}

/*void read_clip_history(Item ** items_start, Item ** items_end, char * file_name, int * current_queue_size, int * flag_inserted, int size_of_clipboard) {
    int line_count = 0, str_len = 0, new_queue_size = 0;
    char * str = NULL, * line = NULL;
    FILE * fp = fopen(file_name, "r");
    Item * new_items_start = NULL, * new_items_end = NULL;
    size_t line_len = 0;

    // read new clipboard
    while((line_count = getline(&line, &line_len, fp))) {
        if(line_count < 0 || (isdigit(line[0]) && isdigit(line[1]) && line[2] == ':')) { // EOF or new record
            // insert into linked list
            if(str) {
                str[strlen(str) - 1] = '\0'; // last character in each record is added newline, so delete it
                if(*items_start && !strcmp(str, (*items_start)->elem)) { // after this point, clipboard doesn't change
                    free(str);
                    free(line);
                    break;
                }
                insert_item(&new_items_start, &new_items_end, str, &new_queue_size, size_of_clipboard, 1);
            }
            // reset the string
            str_len = 0;
            free(str);
            str = NULL;
            // if EOF was reached
            if(line_count < 0) {
                free(line);
                break;
            }
        } else str_append(&str, line, &str_len);
        free(line);
        line_len = 0;
        line = NULL;
    }
    fclose(fp);
    // attach newly added items to the start of the queue(if they exist)
    if(*items_start) {
        // connect the end of the new list to the start of old
        (*items_start)->prev = new_items_end;
        if(new_items_end) { // new list exists
            new_items_end->next = *items_start;
            *items_start = new_items_start;
        }
        // check if number of new elements exceeds the allowed clipboard size
        if(*current_queue_size + new_queue_size > size_of_clipboard) {
            Item * tmp = *items_end;
            for(int i = *current_queue_size + new_queue_size; i > size_of_clipboard; i--) {
                tmp = tmp->prev;
                free(tmp->next->elem);
                free(tmp->next);
            }
            *items_end = tmp;
            *current_queue_size = size_of_clipboard;
        } else
            *current_queue_size += new_queue_size;
    } else {
        *items_start = new_items_start;
        *items_end = new_items_end;
        *current_queue_size = new_queue_size;
    }
    if(new_queue_size) // clipboard should be updated
        *flag_inserted = 1;
}*/

void read_clip_history(Item ** items_start, Item ** items_end, char * file_name, int * current_queue_size, int size_of_clipboard) {
    int line_count = 0, str_len = 0, new_queue_size = 0;
    char * str = NULL, * line = NULL;
    FILE * fp = fopen(file_name, "r");
    Item * new_items_start = NULL, * new_items_end = NULL;
    size_t line_len = 0;

    // read new clipboard
    while((line_count = getline(&line, &line_len, fp))) {
        if(line_count < 0 || (isdigit(line[0]) && isdigit(line[1]) && line[2] == ':')) { // EOF or new record
            // insert into linked list
            if(str) {
                str[strlen(str) - 1] = '\0'; // last character in each record is added newline, so delete it
                insert_item(&new_items_start, &new_items_end, str, &new_queue_size, size_of_clipboard, 1);
            }
            // reset the string
            str_len = 0;
            free(str);
            str = NULL;
            // if EOF was reached
            if(line_count < 0) {
                free(line);
                break;
            }
        } else str_append(&str, line, &str_len);
        free(line);
        line_len = 0;
        line = NULL;
    }
    fclose(fp);
    // attach newly added items to the start of the queue(if they exist)
    free_only_queue(*items_start);
    *items_start = new_items_start;
    *items_end = new_items_end;
    *current_queue_size = new_queue_size;
}

// AVL related
int my_compare(const void * a, const void * b) {
    Item * arg1 = (Item *) a;
    Item * arg2 = (Item *) b;
    if(strcmp(arg1->elem, arg2->elem) < 0) return -1;
    if(strcmp(arg1->elem, arg2->elem) > 0) return 1;
    else return 0;
}

void * my_print(void * key) {
    printf("%s\n", ((Item *) key)->elem);
}

Item * create_item(char * str) {
    Item * new_item = (Item *) malloc(sizeof(Item));
    char * new_str = (char *) malloc((strlen(str) + 1) * sizeof(char));
    memset(new_str, '\0', strlen(str) + 1);
    strncpy(new_str, str, strlen(str));
    new_item->elem = new_str;
    new_item->prev = NULL;
    new_item->next = NULL;
    return new_item;
}

int insert_item(Item ** items_start, Item ** items_end, char * str, int * current_queue_size, int size_of_clipboard, int flag_reversed) {
    Item * new_item, * item_exists;
    if(!str || !strlen(str)) return 0; // string is empty
    new_item = create_item(str);
    // if there are no items in the queue
    if(!(item_exists = find_item(*items_start, new_item->elem))) {
        if(!(*items_start)) {
            *items_start = new_item;
            *items_end = new_item;
        } else {
            // delete the last item, if it is an overflow of clipboard
            if(*current_queue_size >= size_of_clipboard)
                delete_item(items_start, items_end, (*items_end)->elem, current_queue_size);
            // insert new item
            if(!flag_reversed) {
                // add to the start of the queue and connect pointers
                new_item->next = *items_start;
                (*items_start)->prev = new_item;
                *items_start = new_item;
            } else {
                // add to the end of the queue and connect pointers
                new_item->prev = *items_end;
                (*items_end)->next = new_item;
                *items_end = new_item;
            }
        }
        (*current_queue_size)++; // increment the size of the queue
    } else {
        free(new_item->elem);
        free(new_item);
        if(item_exists == *items_start) return 0;
        delete_item(items_start, items_end, item_exists->elem, current_queue_size);
        insert_item(items_start, items_end, str, current_queue_size, size_of_clipboard, flag_reversed);
    }
    return 1;
}

void delete_item(Item ** items_start, Item ** items_end, char * str, int * current_queue_size) {
    Item * it = create_item(str);
    Item * found_item = find_item(*items_start, it->elem);
    // if such a key exists, delete it
    if(found_item) {
        // change start and end pointers(if needed)
        if(*items_start == found_item)
            *items_start = found_item->next;
        if(*items_end == found_item)
            *items_end = found_item->prev;
        // change next and prev pointers
        if(found_item->prev)
            found_item->prev->next = found_item->next;
        if(found_item->next)
            found_item->next->prev = found_item->prev;
        free(found_item->elem);
        free(found_item);
        (*current_queue_size)--; // decrement the size of the queue
    }
    free(it);
}

Item * find_item(Item * items_start, char * str) {
    Item * tmp = items_start;
    while(tmp) {
        if(!strcmp(tmp->elem, str)) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

void iterate_n_print(Item * items_start) {
    int counter = 0;
    Item * tmp = items_start;
    while(tmp) {
        printf("%d:\n", counter);
        printf("%s\n", tmp->elem);
        tmp = tmp->next;
    }
}

void free_queue(Item * items_start, char ** args_to_free, int size) {
    free_only_queue(items_start);
    for(int i = 0; i < size; i++)
        free(args_to_free[i]);
    free(args_to_free);
}

void free_only_queue(Item * items_start) {
    Item * tmp = items_start, * next;
    while(tmp) {
        next = tmp->next;
        free(tmp->elem);
        free(tmp);
        tmp = next;
    }
}

// misc
int str_to_int(char * str) {
    int mult = 1, num = 0;
    for(int i = strlen(str) - 1; i >= 0; i--) {
        if(!isdigit(str[i])) continue;
        num += ((int) str[i] - 48) * mult;
        mult *= 10;
    }
    return num;
}

void str_reverse(char * str, int count) {
	char tmp;
	for(int i = count - 1, j = 0; i > j; i--, j++) {
		tmp = str[i];
		str[i] = str[j];
		str[j] = tmp;
	}
}

char * int_to_str(int num) {
    char * str = NULL;
	int para_num = num, count = 0, len = 1;
	char tmp;
	while(para_num) {
        if(count == len - 1)
            str = (char *) realloc(str, (len *= 2) * sizeof(char));
		str[count] = (char) (para_num % 10 + 48);
		para_num /= 10;
		count++;
	}
    str[count] = '\0';
	str_reverse(str, count);
    return str;
}

char * str_copy(char * str) {
    char * res_str = (char *) malloc((strlen(str) + 1) * sizeof(char));
    strncpy(res_str, str, strlen(str));
    res_str[strlen(str)] = '\0';
    return res_str;
}

void free_2d_array(char ** array, int size) {
    for(int i = 0; i < size; i++)
        free(array[i]);
    free(array);
}

void str_append(char ** str1, char * str2, int * len1) {
	int mult = 1, flag = 0, new_size;
	if(*len1 == 0) {
		new_size = strlen(str2) + 1;
		*len1 = 1;
		flag = 1;
	} else
		new_size = strlen(*str1) + strlen(str2) + 1;
	for(int i = 0; i < 100; i++) {
		if(*len1 * mult > new_size) break;
		mult *= 2;
	}
	if(mult > 1)
		*str1 = (char *) realloc(*str1, (*len1 *= mult) * sizeof(char));
	if(flag) {
		strncpy(*str1, str2, strlen(str2));
		(*str1)[strlen(str2)] = '\0';
	} else
		strncat(*str1, str2, strlen(str2));
}
