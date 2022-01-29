/**************************************************************
* Class:  CSC-415-02 Fall 2021
* Name: Jade Simien
* Student ID: 920258687
* GitHub ID: JadeS01
* Project: Assignment 4 – Word Blast
*
* File: simien_jade_HW4_main.c
*
* Description: A C program that takes a filename and number for threads
*               before using the amount of threads specified to call a
*               function that parses through every word of the given
*               text and stores it in a struct/map if a word is 6+
*               characters long. If a certain word occurs multiple times
*               then the specific word's struct's frequency element
*               increments. This program aims to demonstrate data
*               parallelism. 
*
**************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define ARRAY_SIZE 1000
#define WORD_LEN    10

int txt;
int portion;
int remain = 0;
int entries = 0;
pthread_mutex_t lock;

// You may find this Useful
char * delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";

/** I'm using a struct to create a "map" with keys being a word and
 *  the value being the number of times a word appears. Then, I've
 *  created a method to populate the elements accordingly.
 */
typedef struct WordMap {
    char *word;
    int freq;
} WordMap;
struct WordMap wordMap[ARRAY_SIZE];

/** I created a function that the threads will utiliz. Following the 
 *  logic shown in the link, I'm using a void pointer function because 
 *  the argument and return values of thread functions are generic and 
 *  can be casted into specific data types.
 *  https://stackoverflow.com/questions/11626786/what-does-void-mean-and-how-to-use-it/11629682
*/
void *wordBlast(void *arg){
    char *buffer;
    char *str;
    int diff;
    buffer = malloc(portion + remain);
    read(txt, buffer, (portion + remain));

    /** strtok_r parses through every word from the text file. */
    while(str = strtok_r(buffer, delim, &buffer)){
        /** This checks if the string has 6+ characters. */
        if(strlen(str) >= 6){
            /** If the key already exists in the wordMap, then the word's frequency
             *  increments. If the key does not exist, then it becomes a new entry
             *  in the map.
             */
            for(int i = 0; i < ARRAY_SIZE; i++){
                /** Strcasecmp() ignores casing in comparisons. */
                diff = strcasecmp(wordMap[i].word, str);
                if(diff == 0){
                    pthread_mutex_lock(&lock);
                    /** Modification of the map's value is a critical section. */
                    wordMap[i].freq++; 
                    pthread_mutex_unlock(&lock);
                    break;
                }
            } 
            if(diff != 0) {
                if(entries < ARRAY_SIZE){
                    pthread_mutex_lock(&lock);
                    /** Adding an entry to the map is a critical section. */
                    strcpy(wordMap[entries].word, str);
                    wordMap[entries].freq++;                        
                    pthread_mutex_unlock(&lock);
                    entries++;
                }
            }
        }
    }
}

int main (int argc, char *argv[])
    {
    char *file;
    // char ch;
    int txtSize;
    int numThreads;
    /** This initializes the wordMap */
    for (int i = 0; i < ARRAY_SIZE; i++){
        wordMap[i].word = malloc(WORD_LEN);
        wordMap[i].freq = 0;
    }

    //***TO DO***  Look at arguments, open file, divide by threads
    //             Allocate and Initialize and storage structures
    
    /** Seeing the arguments */
    if(argc < 3){
        printf("No file given and/or thread number given.");
        return(1);
    } else {
        file = argv[1];
        numThreads = strtol(argv[2], NULL, 10); // convert string to a number
        // printf("File: %s\n", file);
    }

    txt = open(file, O_RDONLY);

    /** Ensuring the file is open */
    if(!txt){
        printf("Failed opening file\n");
        return(1);
    }

    txtSize = lseek(txt, 0, SEEK_END);
    // printf("%d\n", txtSize);

    /** We want to divide the file evenly among threads so they have the similar
     *  amount of data to handle.
     */
    lseek(txt, 0, SEEK_SET);
    portion = (txtSize / numThreads); 
    
    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************
    // *** TO DO ***  start your thread processing
    //                wait for the threads to finish

    /** Initialize the mutex lock and threads requested. */
    pthread_mutex_init(&lock, NULL);
    pthread_t thread[numThreads];

    for(int i = 0; i < numThreads; i++){
        /** This calculates the remaining data when the index is at the last
         *  thread.
         */
        if(i == numThreads - 1){
            remain = txtSize % numThreads;
         }
        pthread_create(&thread[i], NULL, wordBlast, (void*)&i);
    }

     /** After threads complete their tasks, they must be joined.*/
    for(int i = 0; i < numThreads; i++){
         pthread_join(thread[i], NULL);
    }

    // ***TO DO *** Process TOP 10 and display

    printf("\n\nWord Frequency Count on %s with %d threads\n", file, numThreads);
    printf("Printing top 10 words 6 characters or more.\n");
    /** Using bubble sort to print out words and frequency. */
    WordMap temp;
    for(int i = 0; i < ARRAY_SIZE; i++){
        for(int j = i + 1; j < ARRAY_SIZE; j++){
            if(wordMap[i].freq < wordMap[j].freq){
                temp = wordMap[i];
                wordMap[i] = wordMap[j];
                wordMap[j] = temp;
            }
        }
    }
    for(int i = 0; i < 10; i++){
        printf("Number %d is %s with a count of %d\n", (i + 1), wordMap[i].word,
            wordMap[i].freq);
    }

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
        {
        --sec;
        n_sec = n_sec + 1000000000L;
        }

    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************

    // ***TO DO *** cleanup
    close(txt);
    /** I used malloc to initialize the word element so all instances must be freed. */
    for(int i = 0; i < ARRAY_SIZE; i++){
        free(wordMap[i].word);
    }
    pthread_mutex_destroy(&lock);
}
