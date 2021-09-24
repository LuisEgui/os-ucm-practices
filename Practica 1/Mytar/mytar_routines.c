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
copynFile(FILE * origin, FILE * destination, int nBytes) {
    int character = 0, copiedBytes = 0;

    if(origin == NULL || destination == NULL)
        return -1;

    while((copiedBytes < nBytes) && (character = getc(origin)) != EOF ) {
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
loadstr(FILE * file) {
    char *buffer = NULL;
    int i = 0, character, size = 0;
    long currentFilePosition;

    if(file == NULL)
        return NULL;
    
    // We check how many memory we need to allocate
    currentFilePosition = ftell(file);

    do {
        character = getc(file);
        size++;
    } while ((character != EOF) && (character != '\0'));
    
    if(fseek(file, currentFilePosition, SEEK_SET) != 0)
        return NULL;

    buffer = (char *) malloc(size * sizeof(char));

    if(buffer == NULL) {
        fprintf(stderr, "Allocation memory error!");
        return NULL;
    }

    do {
        character = getc(file);
        buffer[i] = character;
        i++;
    } while((character != '\0') && (character != EOF));

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
readHeader(FILE * tarFile, int *nFiles) {
    stHeaderEntry* headers = NULL;
    int i = 0;

    if(tarFile == NULL)
        return NULL;
    
    if(!fread(nFiles, sizeof(int), 1, tarFile))
        return NULL;

    headers = (stHeaderEntry*) malloc(*nFiles * sizeof(stHeaderEntry));

    if(headers == NULL)
        return NULL;

    for(i = 0; i < *nFiles; i++) {
        headers[i].name = loadstr(tarFile);

        if(headers[i].name == NULL)
            goto error_asign;
        if(!fread(&headers[i].size, sizeof(unsigned int), 1, tarFile))
            goto error_asign;
    }

    return headers;

error_asign:
    for(int j = 0; j < i; j++)
            free(headers[j].name);
    free(headers);
    return NULL;
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
createTar(int nFiles, char *fileNames[], char tarName[]) {
    FILE* tarFile = NULL;
    int headerSize = sizeof(int) + (nFiles * sizeof(unsigned int));
    stHeaderEntry *header = NULL;
    char endStr = '\0';
    int i = 0;

    header = (stHeaderEntry *) malloc(nFiles * sizeof(stHeaderEntry));

    if(header == NULL)
        return EXIT_FAILURE;
    
    tarFile = fopen(tarName, "w");

    if(tarFile == NULL) {
        fprintf(stderr, "Failed to open \"%s\" file!\n", tarName);
        goto error_path;
    }

    for(i = 0; i < nFiles; i++) {
        header[i].name = (char *) malloc(strlen(fileNames[i]) + 1);
        
        if(header[i].name == NULL) {
            for(int j = 0; j < i; j++)
                free(header[j].name);
            free(header);
            return EXIT_FAILURE;
        }
        
        strcpy(header[i].name, fileNames[i]);
        headerSize += strlen(fileNames[i]) + 1;
    }

    headerSize *= sizeof(char);

    if(fseek(tarFile, headerSize, SEEK_SET) != 0)
        goto error_path;

    // For each file we copy the data inside each of them, and store the file name and size.
    for(int i = 0; i < nFiles; i++) {
        FILE* inputFile = fopen(fileNames[i], "r");
        
        if(inputFile == NULL)
            goto error_path;
        
        header[i].size = copynFile(inputFile, tarFile, INT_MAX);
        fclose(inputFile);
    }

    // Store header into the tar's file
    if(fseek(tarFile, 0, SEEK_SET) != 0)
        goto error_path;

    fwrite(&nFiles, sizeof(int), 1, tarFile);

    for(int i = 0; i < nFiles; i++) {
        fwrite(header[i].name, strlen(header[i].name), 1, tarFile);
        fwrite(&endStr, sizeof(char), 1, tarFile);
        fwrite(&header[i].size, sizeof(unsigned int), 1, tarFile);
    }

    fprintf(stdout, "Mtar \"%s\" file created successfuly\n", tarName);

    for(int i = 0; i < nFiles; i++)
        free(header[i].name);

    free(header);
    fclose(tarFile);

    return EXIT_SUCCESS;

error_path:
    for(int i = 0; i < nFiles; i++)
        free(header[i].name);
    free(header);
    fclose(tarFile);
    return EXIT_FAILURE;
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
extractTar(char tarName[]) {
    FILE* tarFile = NULL, *outputFile = NULL;
    int nFiles = 0;
    stHeaderEntry* header = NULL;

    tarFile = fopen(tarName, "r");

    if(tarFile == NULL) {
        fprintf(stderr, "Error while opening file \"%s\" file!\n", tarName);
        return EXIT_FAILURE;
    }

    header = readHeader(tarFile, &nFiles);

    if(header == NULL) {
        fprintf(stderr, "Error while reading file \"%s\" file!\n", tarName);
        return EXIT_FAILURE;
    }
    
	for(int i = 0; i < nFiles; i++) {
        outputFile = fopen(header[i].name, "w");
        int ret;

        if(outputFile == NULL) {
            fprintf(stderr, "Error while opening file \"%s\" file!\n", header[i].name);
            goto error_path;
        }

        ret = copynFile(tarFile, outputFile, header[i].size);

        if(ret < 0) {
            fprintf(stderr, "Error copying \"%s\" file!\n", header[i].name);
            fclose(outputFile);
            goto error_path;
        }
        
        fprintf(stdout, "\"%s\" file extracted successfuly from \"%s\"!\n", header[i].name, tarName);
        fclose(outputFile);
    }

    fclose(tarFile);

    return EXIT_SUCCESS;

error_path:
    for(int i = 0; i < nFiles; i++)
        free(header[i].name);
    free(header);
    fclose(tarFile);
    return EXIT_FAILURE;
}
