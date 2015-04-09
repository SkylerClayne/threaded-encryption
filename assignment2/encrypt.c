/**
 * Family Name:     Layne
 * Given Name:      Skyler
 * Section:         Z
 * Student Number:  212166906
 * CSE Login:       cse23170
 *
 * Encryption software in the most obscure way
 */

// ONLY USED ON MAC
#ifdef __APPLE__
#  define error printf
#endif
// ONLY USED ON MAC

#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#define TEN_MILLIS_IN_NANOS 10000000

/* THE STRUCTURE OF A BUFFER ITEM */
typedef  struct {
    char  data ;
    off_t offset ;
    char state;
} BufferItem ;

/* GLOBAL VARIABLES */
BufferItem *buffer;
struct timespec t;
FILE *file_in, *file_out;
unsigned int seed = 0;
int bufSize;
int activeIn = 0;
int activeWork = 0;
int activeOut = 0;

/* PROTOFUNCTIONS */
void *in_f(void *arg);
void *work_f(void *arg);
void *out_f(void *arg);
int getFirstEmptyBuffer();
int getFirstWorkBuffer();
int getFirstOutBuffer();

/* MUTEX */
pthread_mutex_t mutex, empty, filled, encode;

/*
 * Thread to control reading file into buffer
 */
void *in_f(void *arg){
    nanosleep(&t, NULL);
    
    /* DECLARATION OF LOCAL VARIABLES */
    int index;
    char character;
    off_t off;

    do {

        pthread_mutex_lock(&mutex);
        index = getFirstEmptyBuffer();
        
        while (index > -1) {
            
            pthread_mutex_lock(&empty);
            off = ftell(file_in);
            character = fgetc(file_in);
            pthread_mutex_unlock(&empty);

            
            if (character == EOF) {
                break;
            } else if (character == '\0'){
               error("Error While Reading from File at offset: %lld, Re Run the program \n", off);
            } else {
                
                /* PLACE THE READ CHARACTER IN THE POSITION AND MOVE STATE ALONG */
                buffer[index].offset = off;
                buffer[index].data = character;
                buffer[index].state = 'w';
                index = getFirstEmptyBuffer();
            }
        }
        pthread_mutex_unlock(&mutex);

    }while (!feof(file_in));
    
    pthread_mutex_lock(&mutex);
    activeIn--;
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(0);
}

/*
 * Thread to control encrypting each character in the buffer
 */
void *work_f(void *arg){
    nanosleep(&t, NULL);

    /* LOCAL VARIABLES */
    int check, workActiveInCheck;
    char character;
    int key = atoi(arg);

    do {
        
        pthread_mutex_lock(&mutex);
        check = getFirstWorkBuffer();

        if(check > -1) {

            character = buffer[check].data;
            
           if (character == EOF) {
               /* SOMEHOW THE CHARACTER AT THAT INDEX IS EOF */
               break;
            } else if (character == '\0'){
                /* SOMEHOW THE CHARACTER AT THAT INDEX WAS NULL */
            }
                /* CHARACTER VALID AND READY TO ENCRYPT/DECRYPT */
                pthread_mutex_lock(&encode);
                if(key >= 0 && character>31 && character<127) character = (((int)character-32)+2*95+key)%95+32;
                else if (key < 0 &&character>31 && character<127 ) character = (((int)character-32)+2*95-(-1*key))%95+32;
                pthread_mutex_unlock(&encode);
            
                /* REPLACE THE CHARACTER IN THE POSITION AND MOVE STATE ALONG */
                buffer[check].data = character;
                buffer[check].state = 'o';
            
        }
        workActiveInCheck = activeIn;
        pthread_mutex_unlock(&mutex);
        
    }while ( workActiveInCheck > 0 || check > -1);
    
    pthread_mutex_lock(&mutex);
    activeWork--;
    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}

/*
 * Thread to control writing the encrypted characters to a file
 */
void *out_f(void *arg){
    nanosleep(&t, NULL);

    /* DECLARATION OF LOCAL VARIABLES */
    int check;
    char character;
    off_t off;
    int workActiveWorkCheck;
    
    do {
        
        pthread_mutex_lock(&mutex);
        check = getFirstOutBuffer();

        if (check > -1) {
            
            /* GET A CHARACTER AND PLACE IN CORRECT FILE POSITION */
            character = buffer[check].data;
            off = buffer[check].offset;
            
            pthread_mutex_lock(&filled);
            if (fseek(file_out, off, SEEK_SET) == -1) {
                fprintf(stderr, "error setting output file position to %u\n", (unsigned int) off);
                exit(-1);
            }
            if (fputc(character, file_out) == EOF) {
                fprintf(stderr, "error writing byte %d to output file\n", character);
                exit(-1);
            }
            pthread_mutex_unlock(&filled);
            
            /* REPLACE THE CHARACTER IN THE POSITION AND MOVE STATE ALONG */
            buffer[check].data = '\0';
            buffer[check].state = 'e';
            buffer[check].offset = 0;
            
        }
        
        workActiveWorkCheck = activeWork;
        pthread_mutex_unlock(&mutex);

    } while ( workActiveWorkCheck > 0 || check > -1) ;
    
    pthread_exit(0);
}

int main(int argc, const char * argv[]) {
    
    /* VARIABLES */
    int key, nIN, nWORK, nOUT, index;
    assert("BEGIN SETUP OF VARIABLES");

    
    assert("BEGIN DEFENSIVE INPUT HANDLING");
    if (argc != 8) error("Usage: encrypt <KEY> <nIN> <nWORK> <nOUT> <file_in> <file_out> <bufSize>");
    if((key=atoi(argv[1])) < -127 || key > 127) error("Encrpyting/decrypting key should be on the reange -127 to 127");
    if((nIN = atoi(argv[2])) < 1) error("Number of input threads should be atleast 1");
    if((nWORK = atoi(argv[3])) < 1) error("Number of encrpyting/decrypting threads should be atleast 1");
    if((nOUT = atoi(argv[4])) < 1) error("Number of outputting threads should be atleast 1");
    if (!(file_in = fopen(argv[5], "r"))) error("could not open input file for reading");
    if (!(file_out = fopen(argv[6], "w"))) error("could not open output file for writing");
    if((bufSize = atoi(argv[7])) < 1) error("The size of the buffer should be atlest 1");
    
    assert("INITIALIZE BUFFER");
    buffer = (BufferItem *) malloc(sizeof (BufferItem) * bufSize);
    for (index = 0; index < bufSize; index++) {
        buffer[index].state = 'e';
    }
    
    assert("INITIALIZE THREADS");
    pthread_t in[nIN], work[nWORK], out[nOUT];
    
    /* ASSERT: SETUP RANDOM WAITING TIME */
    t.tv_sec = 0;
    t.tv_nsec = rand_r(&seed)%(TEN_MILLIS_IN_NANOS+1);
    
    
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&empty, NULL);
    pthread_mutex_init(&filled, NULL);
    pthread_mutex_init(&encode, NULL);

    assert("BEGIN SETUP ON THREADS");
    for (index = 0; index < nIN; index++) {
        activeIn++;
        pthread_create(&in[index], NULL, in_f, &argv[5]);
    }

    for (index = 0; index < nWORK; index++) {
        activeWork++;
        pthread_create(&work[index], NULL, work_f, argv[1]);
    }

    for (index = 0; index < nOUT; index++) {
        pthread_create(&out[index], NULL, out_f, &argv[6]);
    }
    
    
    for (index = 0; index < nIN; index++) {
        pthread_join(in[index], NULL);
    }
    assert("all IN threads joined\n");

    for (index = 0; index < nWORK; index++) {
        pthread_join(work[index], NULL);
    }
    assert("all WORK threads joined\n");

    for (index = 0; index < nOUT; index++) {
        pthread_join(out[index], NULL);
    }
    assert("all OUT threads joined\n");

    
    assert("CLOSE FILES AND FREE BUFFER");
    fclose(file_in);
    fclose(file_out);
    free(buffer);
    
    return 0;
}

/*
 * Get the index of the first empty slot in the buffer.
 */
int getFirstEmptyBuffer(){
    int i;
    for (i = 0; i < bufSize; i++) {
        if(buffer[i].state == 'e')
        {
            return i;
        }
    }
    return -1;
}

/*
 * Get the first available index to be encrypted.
 */
int getFirstWorkBuffer(){
    int i = 0;
    for (i = 0; i < bufSize; i++) {
        if(buffer[i].state == 'w')
        {
            return i;
        }
    }
    return -1;
    
}

/*
 * Get the first index of encrypted to be output.
 */
int getFirstOutBuffer(){
    int i;
    for (i = 0; i < bufSize; i++) {
        if(buffer[i].state == 'o')
        {
            return i;
        }
    }
    return -1;
    
}

