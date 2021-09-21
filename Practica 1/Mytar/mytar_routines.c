#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	int character = 0;
    int copiedBytes = 0;

    if(origin == NULL || destination == NULL)
        return -1;

    while((character = getc(origin)) != EOF && copiedBytes < nBytes) {
        putc(character, destination);
        copiedBytes++;
    }

	return copiedBytes;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE * file)
{
	char *buffer = NULL;
    char character;
    int i = 0;

    if(file == NULL)
        return NULL;

    buffer = malloc(STR_MAX * sizeof(char));

    if(buffer == NULL)
        return NULL;

    while((character = getc(file)) != '\0' && character != EOF) {
        buffer[i] = character;
        i++;
    }

    buffer[i] = '\0';

	return buffer;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE * tarFile, int *nFiles)
{
	stHeaderEntry* headers = NULL;

    if(tarFile == NULL)
        return NULL;
    
    if(!fread(nFiles, sizeof(int), 1, tarFile))
        return NULL;

    headers = (stHeaderEntry*) malloc(*nFiles * sizeof(stHeaderEntry));

    if(headers == NULL)
        return NULL;

    for(int i = 0; i < *nFiles; i++) {
        headers[i].name = loadstr(tarFile);
        if(headers[i].name == NULL) {
            for(int j = 0; j < i; j++)
                free(headers[j].name);
            free(headers);
            return NULL;
        }
        if(!fread(&headers[i].size, sizeof(unsigned int), 1, tarFile)) {
            for(int j = 0; j < i; j++)
                free(headers[j].name);
            free(headers);
            return NULL;
        }
    }

	return headers;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	FILE* tarFile = NULL;
    int headerSize = sizeof(int) + (nFiles * sizeof(unsigned int));
    stHeaderEntry *header = NULL;

    header = (stHeaderEntry *) malloc(nFiles * sizeof(stHeaderEntry));

    if(header == NULL)
        return EXIT_FAILURE;
    
    tarFile = fopen(tarName, "w");

    if(tarFile == NULL) {
        free(header);
        return EXIT_FAILURE;
    }

    for(int i = 0; i < nFiles; i++) {
        header[i].name = (char *) malloc(strlen(fileNames[i]) * sizeof(char) + 1);
        if(header[i].name == NULL) {
            for(int j = 0; j < i; j++) {
                free(header);
                return EXIT_FAILURE;
            }
        }
        strcpy(header[i].name, fileNames[i]);
        headerSize += strlen(fileNames[i]) * sizeof(char) + 1;
    }

    if(fseek(tarFile, headerSize, SEEK_SET) != 0) {
        for(int i = 0; i < nFiles; i++) {
            free(header);
        }
        fclose(tarFile);
        return EXIT_FAILURE;
    }

    // For each file we copy the data inside each of them, and store the file name and size.
    for(int i = 0; i < nFiles; i++) {
        FILE* inputFile = fopen(fileNames[i], "r");
        if(inputFile == NULL) {
            for(int j = 0; j < nFiles; j++) {
                free(header);
            }
            return EXIT_FAILURE;
        }
        
        header[i].size = copynFile(inputFile, tarFile, INT_MAX);
        fclose(inputFile);
    }

    // Store header into the tar's file
    if(fseek(tarFile, 0, SEEK_SET) != 0) {
        for(int i = 0; i < nFiles; i++) {
            free(header);
        }
        fclose(tarFile);
        return EXIT_FAILURE;
    }

    fwrite(&nFiles, sizeof(unsigned int), 1, tarFile);

    for(int i = 0; i < nFiles; i++) {
        fwrite(header[i].name, strlen(header[i].name), 1, tarFile);
        fwrite("\0", sizeof(char), 1, tarFile);
        fwrite(&header[i].size, sizeof(unsigned int), 1, tarFile);
    }

    fprintf(stdout, "Mtar \"%s\" file created successfuly\n", tarName);

    for(int i = 0; i < nFiles; i++)
        free(header[i].name);

    free(header);
    
    fclose(tarFile);

	return EXIT_SUCCESS;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
	FILE* tarFile = NULL;
    int nFiles = 0;
    stHeaderEntry* header = NULL;

    tarFile = fopen(tarName, "r");

    if(tarFile == NULL)
        return EXIT_FAILURE;

    header = readHeader(tarFile, &nFiles);

    if(header == NULL) {
        return EXIT_FAILURE;
    }
    
	for(int i = 0; i < nFiles; i++) {
        FILE* outputFile = fopen(header[i].name, "w");
        int ret;
        if(outputFile == NULL) {
            for(int i = 0; nFiles; i++)
                free(header[i].name);
            free(header);
            return EXIT_FAILURE;
        }

        ret = copynFile(tarFile, outputFile, header[i].size);

        if(ret < 0) {
            for(int i = 0; nFiles; i++)
                free(header[i].name);
            free(header);
            return EXIT_FAILURE;
        }

        fclose(outputFile);
    }

    return EXIT_SUCCESS;
}
