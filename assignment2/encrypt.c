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

/* VARIABLES AND MUTEX */
/* GLOBAL VARIABLES */
BufferItem *buffer;
struct timespec t;
FILE *file_in, *file_out;
unsigned int seed = 0;
volatile int bufSize;
volatile int check;
volatile int activeIn = 0;
volatile int activeWork = 0;
volatile int activeOut = 0;


/* MUTEX */
pthread_mutex_t mutex, empty, filled, encode;

void *in_f(void *arg){
    
    int index;
    char character;
    nanosleep(&t, NULL);
    
    pthread_mutex_lock(&mutex);
    activeIn++;
    pthread_mutex_unlock(&mutex);
    assert("ACTIVE IN THREAD VARIABLE INCREMENTED");


    do {
        pthread_mutex_lock(&empty);
        pthread_mutex_lock(&mutex);
        index = getFirstEmptyBuffer();
        if (index != -1) {

            off_t off = ftell(file_in);
            character = fgetc(file_in);
            
            if (character == EOF) {
                break;
            } else if (character == '\0'){
                printf("Error While Reading from File at offset = %lld Re Run the program ", off);
                error("Read Error Re Run the program at offset = %lld", off);
            } else {
                // all is good
                buffer[index].offset = off;
                buffer[index].data = character;
                buffer[index].state = 'w';
            }
        }
        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&encode);

    }while (!feof(file_in));
    
    pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&encode);
    
    pthread_mutex_lock(&mutex);
    activeIn--;
    pthread_mutex_unlock(&mutex);
    assert("RELEASE ACTIVE IN THREAD VARIABLE");

    nanosleep(&t, NULL);
    
    pthread_exit(0);
}

void *work_f(void *arg){
    int check;
    char character;
    nanosleep(&t, NULL);
    
    pthread_mutex_lock(&mutex);
    activeWork++;
    printf("active in: %d\n",activeWork);
    check = getFirstWorkBuffer();

    pthread_mutex_unlock(&mutex);
    assert("ACTIVE WORK THREAD VARIABLE INCREMENTED");
    
    
    do {
        pthread_mutex_lock(&encode);
        pthread_mutex_lock(&mutex);

        
        if (check > -1) {
            assert("FOUND A CHARACTER READY FOR ENCRYPTION");

            character = buffer[check].data;
            
           if (character == EOF) {
               assert("SOMEHOW THE CHARACTER AT THAT INDEX IS EOF");
               break;
            } else if (character == '\0'){
                assert("SOMEHOW THE CHARACTER AT THAT INDEX WAS NULL");
            } else {
                assert("CHARACTER VALID AND READY TO ENCRYPT/DECRYPT");
                // encrpyt
                if(atoi(arg) >= 0 && character>31 && character<127)character = (((int)character-32)+2*95+atoi(arg))%95+32;
                
                // decrypt
                else if (atoi(arg) < 0 &&character>31 && character<127 )character = (((int)character-32)+2*95-atoi(arg))%95+32;
                
                buffer[check].data = character;
                buffer[check].state = 'o';

            }
            nanosleep(&t, NULL);
        }
        
        check = getFirstWorkBuffer();
        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&filled);

    } while (activeIn > 0 || check > -1);

    
    pthread_mutex_lock(&mutex);
    activeWork--;
    printf("active work: %d\n",activeWork);
    pthread_mutex_unlock(&mutex);
    
   // pthread_mutex_unlock(&filled);
    pthread_mutex_unlock(&encode);


    nanosleep(&t, NULL);
    
    pthread_exit(0);
}

void *out_f(void *arg){
    int check;
    nanosleep(&t, NULL);
    check = getFirstOutBuffer();

    do {
        pthread_mutex_lock(&filled);
        pthread_mutex_lock(&mutex);
        
        if (check > -1) {
            char character = buffer[check].data;
            
            if (character == EOF) {
                break;
            } else if (character == '\0'){
                break;
            } else {
                // all is good
                
                if(buffer[check].data == '\0')
                    printf("%lld ", buffer[check].offset);
                
                if (fseek(file_out, buffer[check].offset, SEEK_SET) == -1) {
                    fprintf(stderr, "error setting output file position to %u\n",(unsigned int) buffer[check].offset);
                    exit(-1);
                }
                if (fputc(buffer[check].data, file_out) == EOF) {
                    fprintf(stderr, "error writing byte %d to output file\n", buffer[check].data);
                    exit(-1);
                }
                
                buffer[check].data = '\0';
                buffer[check].state = 'e';
                buffer[check].offset = 0;
                check++;
            }
            nanosleep(&t, NULL);
        }
        
        check = getFirstOutBuffer();

        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&empty);

    } while (activeWork > 0 || check > -1);
    
    printf("the end has been reached?\n");

    
  //  pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&empty);
    pthread_mutex_unlock(&filled);


    nanosleep(&t, NULL);

    pthread_exit(0);
}

int main(int argc, const char * argv[]) {
    
    /* VARIABLES */
    int key, nIN, nWORK, nOUT, index;
    
    
    
    /* ASSERT: BEGIN DEFENSIVE INPUT HANDLING */
    if (argc != 8) error("Usage: encrypt <KEY> <nIN> <nWORK> <nOUT> <file_in> <file_out> <bufSize>");
    if((key=atoi(argv[1])) < -127 || key > 127) error("Encrpyting/decrypting key should be on the reange -127 to 127");
    if((nIN = atoi(argv[2])) < 1) error("Number of input threads should be atleast 1");
    if((nWORK = atoi(argv[3])) < 1) error("Number of encrpyting/decrypting threads should be atleast 1");
    if((nOUT = atoi(argv[4])) < 1) error("Number of outputting threads should be atleast 1");
    if (!(file_in = fopen(argv[5], "r"))) error("could not open input file for reading");
    if (!(file_out = fopen(argv[6], "w"))) error("could not open output file for writing");
    if((bufSize = atoi(argv[7])) < 1) error("The size of the buffer should be atlest 1");
    /* ASSERT: END DEFENSIVE INPUT HANDLING */
    
    
    printf("bufsize: %d\n", bufSize);
    /* INITIALIZE BUFFER */
    buffer = (BufferItem *) malloc(sizeof (BufferItem) * bufSize);
    for (index = 0; index < bufSize; index++) {
        buffer[index].state = 'e';
    }
    
    /* THREADS AND ATTRIBUTES */
    pthread_t in[nIN], work[nWORK], out[nOUT];
    
    /* ASSERT: SETUP RANDOM WAITING TIME */
    t.tv_sec = 0;
    t.tv_nsec = rand_r(&seed)%(TEN_MILLIS_IN_NANOS+1);
    
    
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&empty, NULL);
    pthread_mutex_init(&filled, NULL);
    pthread_mutex_init(&encode, NULL);
    
    
    /* ASSERT: BEGIN SETUP ON THREADS */
    for (index = 0; index < nIN; index++) {
       // activeIn++;
        pthread_create(&in[index], NULL, in_f, &argv[5]);
        nanosleep(&t, NULL);
    }

    for (index = 0; index < nWORK; index++) {
        pthread_create(&work[index], NULL, work_f, argv[1]);
        nanosleep(&t, NULL);
    }

    

    for (index = 0; index < nOUT; index++) {
        pthread_create(&out[index], NULL, out_f, &argv[6]);
        nanosleep(&t, NULL);
    }
    /* ASSERT: END SETUP ON THREADS */
    

    
    
    /* ASSERT: BEGIN JOINING THREADS */
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
    /* ASSERT: END JOINING THREADS */
    assert("all OUT threads joined\n");

    
    
    /* ASSERT: CLOSE FILES */
    fclose(file_in);
    fclose(file_out);
    /* ASSERT: FILES CLOSED */
    free(buffer);
    
    return 0;
}

/* Check for any empty buffer slots
	Returns 1 if there is empty buffer else returns 0 */
int hasEmptyBuffer(){
    int i = 0, count = 0;
    while(i < bufSize){
        if(buffer[i].state != 'e')
        {
            count++;
        }
        i++;
    }
    
    
    if(bufSize-count > 0)
        return 1;
    else
        return 0;
}

/* Returns -1 if Buffer is not empty or else Avaiable buffer index */
int getFirstEmptyBuffer(){
    int ret = -1;
    if(hasEmptyBuffer())
    {
        int i = 0;
        while(i < bufSize){
            if(buffer[i].state == 'e')
            {
                return i;
            }
            i++;
        }
    }
    return ret;
}

/* getFirstWorkBuffer() modifies the buffer state (getFirstWorkBuffer() reserves the buffer slot for the current thread)
	returns index of first buffer slot in WORK state else returns -1*/
int getFirstWorkBuffer(){
    int i = 0;
    while(i < bufSize){
        if(buffer[i].state == 'w')
        {
            buffer[i].state = 'W';
            return i;
        }
        
        i++;
    }
    return -1;
    
}

/* getFirstOutBuffer() modifies the buffer state (getFirstOutBuffer() reserves the buffer slot for the current thread)
	returns index of first buffer slot in OUT state else returns -1 */
int getFirstOutBuffer(){
    int i = 0;
    while(i < bufSize){
        if(buffer[i].state == 'o')
        {
            buffer[i].state = 'O';
            return i;
        }
        
        i++;
    }
    return -1;
    
}

