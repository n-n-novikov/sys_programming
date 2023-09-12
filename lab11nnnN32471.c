#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

extern int errno;

char* debug, present_opts = 0;

void help_msg() {
	printf("You can run program: ./lab11nnnN32471 [option] <start_directory> what_you_need_to_find(0xhh[hh*])\nThe program will find what_you_need_to_find bytes in the directory\nOptions:\n\t-h, --help\t show this message\n\t-v, --version\tshow the version of a program\n\n");
}

void v_msg() {
	printf("Version 1.0 beta\nNikolai Novikov, N32471, 2\n\n");
}

void options(int argc, char** argv) {
	int input;
	int option_index = 0;
	static struct option long_options[] = {
                   {"help", no_argument, 0, 0},
                   {"version", no_argument, 0, 0},
                   {0, 0, 0, 0}
               };
    while ((input = getopt_long(argc, argv, "hv", long_options, &option_index)) != -1) {
    	present_opts++;
    	switch (input) {
    	case 0:
    		if (option_index == 0) help_msg();
    		else if (option_index == 1) v_msg();
    		break;
    	case 'h':
    		help_msg();
    		break;
    	case 'v':
    		v_msg();
    		break;
    	default:
    		printf("There is no such option!\n\n");
    		help_msg();
    	}
    }
}

void file_search(char* name, char* bytes){
	FILE *fp = fopen(name, "r");
    if (!fp) {
       perror("fopen");
       return;
    }

    char exists = 0;

    char* line = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&line, &len, fp)) != -1) {
    	if (strstr(line, bytes) != NULL) {
    		printf("Found %s in %s\n", bytes, name);
    		if (debug) fprintf(stderr, "line: \"%s\"\n", line);
       		break;
    	}
    }

    free(line);
    fclose(fp);
}

void go_to_dir(char* name, char* bytes) {
	if (debug) fprintf(stderr, "now in %s\n", name);
	DIR* dir;
	struct dirent *entry;
	dir = opendir(name);
	if (!dir) {
		perror("opendir");
		printf("called %s\n", name);
       	return;
    }
    
    char path[PATH_MAX] = {0};
    while ((entry = readdir(dir)) != NULL) {
    	sprintf(path, "%s/%s", name, entry->d_name);
    	if (debug) fprintf(stderr, "	path %s -> %s\n", name, entry->d_name);
    	if (entry->d_type == DT_REG) {
    		file_search(path, bytes);
    	}
    	else if ((entry->d_type == DT_DIR) && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
    		if (debug) fprintf(stderr, "going to dir:   %s\n", entry->d_name);
    		go_to_dir(path, bytes);
    		if (debug) fprintf(stderr, "after returning from %s current path is %s\n", path, name);
    	}
    }
   	if (debug) fprintf(stderr, "closing dir %s\n", name);
    closedir(dir);
}


//------------------------------------------------------------

int main(int argc, char** argv) {
	debug = getenv("LAB11DEBUG");
	printf("\n\n");
	if (argc < 2){
		printf("Error: program requires arguments!\n");
		printf("Usage: ./lab11nnnN32471 [options] <directory> <bytes(0x[hh*])>");
		return -1;
	}

	options(argc, argv);

	if ((argc == 2) && (present_opts) || (argc == 3) && (present_opts == 2)) return 0;
	else if ((argc == 2) && !(present_opts) || (argc == 3) && (present_opts == 1)) {
		printf("Error: program requires more arguments!\n");
		printf("Usage: ./lab11nnnN32471 [options] <directory> <bytes(0x[hh*])>\n");
		return -1;
	}

	char* name = argv[argc-2];
	char* byte = argv[argc-1];
	int len = strlen(byte);
	if (len % 2 != 0) {
		printf("Error: not enough bytes!\n");
		help_msg();
		return 1;
	}

	char* bytes = (char*) malloc(len/2);
	if (bytes == NULL) {
		printf("Error: couldn't allocate memory. Try again\n");
		return 1;
	}
	int ind = 0;
	for (int i = 2; i < len; i += 2) {
		byte[i] = tolower(byte[i]);
		byte[i+1] = tolower(byte[i+1]);
		char n = 0;
		if ((byte[i] > 'f') || (byte[i+1] > 'f')) {
			printf("Error: invalid byte %c%c\n", byte[i], byte[i+1]);
			return 1;
		}
		switch (byte[i]) {
			case 'a': n += 10 * 16;
				break;
			case 'b': n += 11 * 16;
				break;
			case 'c': n += 12 * 16;
				break;
			case 'd': n += 13 * 16;
				break;
			case 'e': n += 14 * 16;
				break;
			case 'f': n += 15 * 16;
				break;
			default:
				n += (byte[i]-48) * 16;
		}
		switch (byte[i+1]) {
			case 'a': n += 10;
				break;
			case 'b': n += 11;
				break;
			case 'c': n += 12;
				break;
			case 'd': n += 13;
				break;
			case 'e': n += 14;
				break;
			case 'f': n += 15;
				break;
			default:
				n += (byte[i+1]-48);
		}
		bytes[ind] = n;
		ind++;
	}

	printf("Search in %s for %s\n", name, bytes);

	go_to_dir(name, bytes);
	printf("done\n");
	free(bytes);

	return 0;
}