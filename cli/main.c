#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 512

int main(int argc, char *argv[])
{
	char ibuffer[BUFFER_SIZE];
	char obuffer[BUFFER_SIZE];

	int device;
	ssize_t read_bytes;

	if (argc == 2) {
		if ((device = open(argv[1], O_RDWR)) == -1) {
			fprintf(stderr, "Could not open file (%d)\n%s\n",
			        errno, strerror(errno));
			return -1;
		}
	} else {
		printf("Missing device filename. Usage:\n"
		       "  %s <device filename>\n", argv[0]);
		return -1;
	}

	do {
		printf(">>> ");
		fgets(ibuffer, BUFFER_SIZE, stdin);
		if (strcmp(ibuffer, "over\n") == 0) {
			break;
		}
		write(device, ibuffer, strlen(ibuffer) + 1);
		if ((read_bytes = read(device, obuffer, BUFFER_SIZE)) >= 0) {
			obuffer[read_bytes] = '\0';
		}
		printf("%s\n", obuffer);
	} while(1);

	close(device);
	return 0;
}
