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


/* MUTEX */
pthread_mutex_t mutex, empty, filled, encode;

void *in_f(void *arg){
    
    int index, count = 0;
    check = 0;
    char character;
    nanosleep(&t, NULL);
    activeIn++;
    
    pthread_mutex_lock(&empty);
    while (!feof(file_in)) {
        int i = -1;
        
        while(hasEmptyBuffer()==0){
            nanosleep(&t, NULL);
        }
        
     //   i = getFirstEmptyBuffer();

      //  if(i != -1){
        pthread_mutex_lock(&mutex);
        printf(" Begin input thread\n");
            off_t off = ftell(file_in);
            character = fgetc(file_in);
            
            if (character == EOF) {
                break;
            } else if (character == '\0'){
                // escape with error
            } else {
                // all is good
                buffer[check].offset = off;
                buffer[check].data = character;
                buffer[check].state = 'w';
                printf("%c", buffer[check].data);
                check++;
            }
        pthread_mutex_unlock(&mutex);
        nanosleep(&t, NULL);
      //  }


    }
    
    pthread_mutex_unlock(&encode);
    nanosleep(&t, NULL);
    activeIn--;
    pthread_exit(0);
}

void *work_f(void *arg){
    char character;
    int index;
    nanosleep(&t, NULL);

    pthread_mutex_lock(&encode);
    printf("  Being encrypting thread\n");
    printf("  with encrypting key: %d\n", atoi(arg));
    
    for (index = 0; index < bufSize; index++) {

        nanosleep(&t, NULL);
        pthread_mutex_lock(&mutex);
        character = buffer[index].data;
        
        // encrpyt
        if(atoi(arg) >= 0 && character>31 && character<127)character = (((int)character-32)+2*95+atoi(arg))%95+32;
        // decrypt
        else if (atoi(arg) < 0 &&character>31 && character<127 )character = (((int)character-32)+2*95-atoi(arg))%95+32;
        
        buffer[index].data = character;
        buffer[index].state = 'o';
        pthread_mutex_unlock(&mutex);
        nanosleep(&t, NULL);

    }
    
    pthread_mutex_unlock(&filled);
    nanosleep(&t, NULL);

    
    pthread_exit(0);
}

void *out_f(void *arg){
    int index;
    nanosleep(&t, NULL);

        pthread_mutex_lock(&filled);
        printf("   Begin output thread.\n");
    
        for (index = 0; index < bufSize; index++) {
            
            nanosleep(&t, NULL);
            pthread_mutex_lock(&mutex);

        
            if(buffer[index].data == '\0') printf("%d ", (unsigned int) buffer[index].offset);
        
            if (fseek(file_out, buffer[index].offset, SEEK_SET) == -1) error("error setting output file position to %u\n",(unsigned int) buffer[index].offset);
            if (fputc(buffer[index].data, file_out) == EOF) error("error writing byte %d to output file\n", buffer[index].data);
        
            buffer[index].data = '\0';
            buffer[index].state = 'e';
            buffer[index].offset = 0;
            check--;
            pthread_mutex_unlock(&mutex);
            nanosleep(&t, NULL);


        }
    
        pthread_mutex_unlock(&empty);
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
        pthread_create(&in[index], NULL, in_f, &argv[5]);
    }

    nanosleep(&t, NULL);

    for (index = 0; index < nWORK; index++) {
        pthread_create(&work[index], NULL, work_f, argv[1]);
    }

    nanosleep(&t, NULL);

    for (index = 0; index < nOUT; index++) {
        pthread_create(&out[index], NULL, out_f, &argv[6]);
    }
    /* ASSERT: END SETUP ON THREADS */
    nanosleep(&t, NULL);

    
    
    /* ASSERT: BEGIN JOINING THREADS */
    for (index = 0; index < nIN; index++) {
        pthread_join(in[index], NULL);
    }
    
    for (index = 0; index < nWORK; index++) {
        pthread_join(work[index], NULL);
    }
    
    for (index = 0; index < nOUT; index++) {
        pthread_join(out[index], NULL);
    }
    /* ASSERT: END JOINING THREADS */
    
    
    
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
    if(hasEmptyBuffer()==1)
    {
        int i = 0;
        while(i < bufSize){
            if(buffer[i].state == 'e')
            {   ret = i;
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

