#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

int main(int argc, char* argv[]) {
	FILE* file = NULL;
	char *buffer;
    unsigned int nbytes = 0;

	if (argc < 3) {
		fprintf(stderr,"Usage: %s <file_name>\n",argv[0]);
		exit(1);
	}

    nbytes = atoi(argv[2]);

	/* Open file */
    file = fopen(argv[1], "r");

	if(file == NULL)
		err(2,"The input file %s could not be opened", argv[1]);
    
    /* Read file byte by byte */
    buffer = (char *) malloc(sizeof(char));

    if(buffer == NULL) {
        fclose(file);
        exit(1);
    }

	while (!feof(file) && nbytes > 0) {
		/* Print byte to stdout */
		if(!fread(buffer, sizeof(char), 1, file)) {
            fclose(file);
            free(buffer);
            exit(1);
        }

        if(!fwrite(buffer, sizeof(char), 1, stdout)) {
            fclose(file);
            free(buffer);
            exit(1);
        }

        nbytes--;
	}

    printf("\n");
    
	fclose(file);
    free(buffer);
	return 0;
}
