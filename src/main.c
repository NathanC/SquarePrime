/*
Nathan Coleman, Programing Languages Project 1
This program evolves n by n magic squares

To use multi-core computational power, this computer
uses the pthread multi-threading library

To compile this with gcc, you need to link it with the
pthread library

The following line is an example compile:

    gcc main.c -o magic_squares.exe -lpthread

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

#define MATRIX(i) population[i].matrix
//Macro to easier access a square in the population

typedef struct{
 int **matrix;

 double fitness;
}m_square;

typedef struct{

    int start;
    int finish;
    unsigned int* seed;

}args;


args w1, w2, w3, w4;
unsigned int seed1, seed2, seed3, seed4;

//used for managing threads

pthread_t worker1, worker2, worker3, worker4;
pthread_t clock_keeper;
//pthread_t keeper_of_the_keys;


int g_dynasty;
int g_generations;
long long g_timer;

char keypress;

int rand_gen (unsigned int *seed);

int run(int generations, int survivors, int loop);
//void keypress_waiter(void *arg);
void *print_timer(void *arg);

void reproduce();
void initialize();
void *initialize_protege(void *arg);
void print_matrix(m_square square);
void analyze();
void *analyze_protege(void *arg);
void spawn(m_square square_a, m_square square_b);

void *mutate_protege(void *arg);
void sort();
void mutate();
void log_file(m_square square);
int  malloc_population();
void free_population();


int size;

int pop_size;
double magic_const;
int ratio;

m_square* population;

int main()
{

    srand((unsigned int) time(NULL));

    seed1 = rand();
    seed2 = rand();
    seed3 = rand();
    seed4 = rand();
    //initializes the various seeds for thread-safe random number generation

    int i, j;
    int outer_loop = 1;

    while(outer_loop == 1)
    {

        outer_loop = 0;

        printf("What size squares would you like? ");

        scanf("%d", &size);

        magic_const = (size*((size*size)+1))/2;

        printf("What population size would you like? ");

        scanf("%d", &pop_size);

        int result;

        result = malloc_population();

        if(result == -1){

            printf("Malloc failed while reserving the population.\n\n");
            outer_loop = 1;
            free_population();
            continue;
            }
        else
            printf("A population of %d squares created, of size %d x %d:\n", pop_size, size, size);

        w1.start= 0;
        w1.finish = pop_size/4;
        w1.seed = &seed1;

        w2.start= pop_size/4;
        w2.finish = pop_size/2;
        w2.seed = &seed2;

        w3.start= pop_size/2;
        w3.finish = pop_size * .75;
        w3.seed = &seed3;

        w4.start= pop_size * .75;
        w4.finish = pop_size;
        w4.seed = &seed4;

        /* Hands out the seeds and divides up the population between the threads.
        This is so later the threads can have valid random numbers,
        and can know what portion of the population to analyze or mutate. */

        ratio = 5;

        printf("\nPreparing population..\n");
        printf("* initializing\n");
        initialize();
        printf("* analyzing\n");
        analyze();
        printf("* sorting\n");
        sort();
        printf("Population prepared.\n");

        int choice = 3;

        int keep_going = 1;

        int g,s,l;
        char a;
        int print_loop;
        int square_count;
        int input_loop;

        while(keep_going == 1)
        {
            //Main menu
            printf("\n~~~~~~~Main Menu~~~~~~~\n");
            printf("Current parameters: size %d by %d, population of %d.\n", size, size, pop_size);
            printf("Would you like to\n1)view the current population\n2)select options for a run\n");
            printf("3)run the program with the default settings?\n");
            printf("4)choose a different square and population size?\n");
            printf("5)quit the program?\nSelection: ");

            scanf("%d", &choice);

            printf("\n");

            switch(choice){

            case 1:
                print_loop = 1;
                square_count = 0;
                while(print_loop == 1)
                {
                    input_loop = 1;

                    system("cls");
                    printf("Square selector.");
                    printf("\nAt square %d of %d", square_count+1, pop_size);
                    print_matrix(population[square_count]);

                    while(input_loop == 1)
                    {
                        input_loop = 0;

                        printf("What square would you like to see next?\n-1 to go back to main menu.\n");
                        scanf("%d", &square_count);
                        square_count--;

                        if(square_count == -2)
                            print_loop = 0;

                        else if(square_count < 0 || square_count >= pop_size)
                        {
                            printf("Square out of range.\n");
                            input_loop = 1;
                        }

                    }

                }

                break;

            case 2:

                printf("\nHow many generations should the program run for? ");
                scanf("%d",&g);
                printf("How many survivors should there be? ");
                scanf("%d",&s);
                printf("Should the program reseed and repeat after %d generations? y/n? ", g);
                scanf(" %c", &a);
                printf("\n");

                if(a == 'y')
                    l = 1;
                else
                    l = 0;

                run(g,s,l);

                printf("\nThe simulation reached generation %d of dynasty %d.\n", g_generations, g_dynasty);

                if(population[0].fitness == 0)
                {
                    printf("\nA magic square was found!");
                    log_file(population[0]);
                }

                else
                    printf("\nNo magic square was found.");

                break;

            case 3:
                run(1000,5,1);

                printf("\nThe simulation reached generation %d of dynasty %d.\n", g_generations, g_dynasty);

                if(population[0].fitness == 0)
                {
                    printf("\nA magic square was found!");
                    log_file(population[0]);
                }

                else
                    printf("\nNo magic square was found.");

                break;

            case 4:
                outer_loop = 1;
                keep_going = 0;
                break;

            case 5:
                keep_going = 0;
                break;

            default:
                printf("Option invalid. Please choose again:\n");
            }

        }

        free_population();
    }

    printf("\nGoodbye.\n");

    return 0;
}


void free_population()
{
    //frees the malloced population
    int i, j;

    for(i = 0; i < pop_size; i++)
    {

        for(j = 0; j < size; j++)
        {
            free(population[i].matrix[j]);
        }

        free(population[i].matrix);
    }

    free(population);

    return;
}


int  malloc_population()
{
    //mallocs the population, returns negative one if malloc fails

    int i, j;

    population = malloc(pop_size*sizeof(m_square));
    if (population == NULL)
            return -1;

    for(i = 0; i < pop_size; i++)
    {
        // zero out the fitness
        population[i].fitness = 0.0;

        // create the matrix
        MATRIX(i) = malloc(size*sizeof(int*));
        if (MATRIX(i) == NULL)
            return -1;

        for(j = 0; j < size; j++)
        {
            MATRIX(i)[j] = malloc(size*sizeof(int));
            if (MATRIX(i)[j] == NULL)
                return -1;

            // zero out the matrix
            for(int x = 0; x < size; x++)
              MATRIX(i)[j][x] = 0;
        }
    }

    return 0;
}

int run(int generations, int survivors, int loop)
{
    //the "driver" function that main calls to evolve a magic square


    printf("\nStarting a run, for %d generations, with %d survivors. Loop = %d\n", generations, survivors, loop);

    ratio = survivors;

    //pthread_create(&keeper_of_the_keys, NULL, keypress_waiter, NULL);
    //This thread was initially used to allow users to press 'q' during a
    //run to go back to menu, but caused a difficult error.
    //The code still actively refers to "keypress", because I do want to go fix this error later

    g_timer = 0;
    long long last_time = 0;
    pthread_create(&clock_keeper, NULL, print_timer, NULL);

    keypress = '\0';

    int i = generations;


    g_dynasty = 0;

    g_generations = 0;

    do
    {
        g_dynasty++;

        for(i = 0; i < generations && keypress != 'q' && population[0].fitness != 0; i++)
        {

            g_generations = i;

           // usleep(1);

            //pthread_mutex_lock(&lock);

            reproduce();
            mutate();
            analyze();
            sort();

            g_generations = i+1;

            //this is used, along with a background timer function
            //to decide wether or not print some info about the run.
            //The background timer(g_timer) globally increments every ~1.5 seconds,
            //and if the local counter (last_time) is less than g_timer, it prints info.


            if(population[0].fitness != 0 && keypress != 'q' && g_timer > last_time)
            {
                last_time = g_timer;
                printf("\nGeneration %d, dynasty %d\n", g_generations, g_dynasty);
                printf("Square prime at fitness of %f\n",population[0].fitness);
            }

        }

        if (population[0].fitness == 0)
            break;

        if (loop == 0)
            break;

        if (keypress == 'q')
            break;

        initialize();
        analyze();
        sort();

    }while(loop == 1 && keypress != 'q');

    //pthread_cancel(keeper_of_the_keys);
    pthread_cancel(clock_keeper);

    usleep(1);

    if (population[0].fitness == 0)
        return 1;

    else
        return 0;
}


void *print_timer(void *arg)
{
pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

while(1)
{
    usleep(999999);
    usleep(500000);
    g_timer++;
}


return 0;
}


/*void keypress_waiter(void *arg)
{

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

    while(1)
    {
   // keypress = getch();
    }


return NULL;
}*/


void sort()
{

    /* A selection sort, that picks the n most-fit squares, where n is the number
    of squares that survive. */

    int r = ratio;
    int i, j;

    for(i = 0; i < r; i++)
    {

        int min = i;

        for(j = i; j <pop_size; j++)
        {
            if(population[j].fitness < population[min].fitness)
                min = j;

        }

        m_square temp1 = population[i];
        m_square temp2 = population[min];
        population[i] = temp2;
        population[min] = temp1;
    }

    return;

}



void *mutate_protege(void *arg)
{
    int i, r;

    int gene1, gene2;

    args *w = (args*)arg;
    //derefrencing the void pointer to a local pointer to a globcal structure,
    //which then is used for this thread to set start, finish, and seed parameters

    int start = w->start;
    int finish = w->finish;
    int r1_a, r1_b,
        r2_a, r2_b;
    int test;

    for(i = start; i < finish; i++)
    {

        test = rand_gen(w->seed)%1000;
        if(test==0)
            continue;
        r1_a = (rand_gen(w->seed))%size;
        r1_b = (rand_gen(w->seed))%size;

        do{
            r2_a = (rand_gen(w->seed))%size;
            r2_b = (rand_gen(w->seed))%size;
        }while(MATRIX(i)[r2_a][r2_b] == MATRIX(i)[r1_a][r1_b]);


        gene1 = MATRIX(i)[r1_a][r1_b];
        gene2 = MATRIX(i)[r2_a][r2_b];

        MATRIX(i)[r2_a][r2_b] = gene1;
        MATRIX(i)[r1_a][r1_b] = gene2;

    }

    return 0;

}

void mutate()
{

    //Dispaches the threads, with their various argument-structures

    pthread_create(&worker1, NULL, mutate_protege, &w1);
    pthread_create(&worker2, NULL, mutate_protege, &w2);
    pthread_create(&worker3, NULL, mutate_protege, &w3);
    pthread_create(&worker4, NULL, mutate_protege, &w4);

    //Waits for the threads to finish
    pthread_join(worker1, NULL);
    pthread_join(worker2, NULL);
    pthread_join(worker3, NULL);
    pthread_join(worker4, NULL);

return;
}


void spawn(m_square square_a, m_square square_b)
{

    int i, j;

    for(i = 0; i < size; i++)
    {
        for( j = 0; j < size; j++)
        {
            square_b.matrix[i][j] = square_a.matrix[i][j];
        }

    }

    square_b.fitness = square_a.fitness;

    return;
}

void reproduce()
{
    //copies the survivors (variable named "ratio") onto the rest of the population

    int i;
    int selection = 0;

    for(i = ratio; i < pop_size; i++)
    {
        spawn(population[selection], population[i]);
        selection++;

        if(selection == ratio)
            selection = 0;
    }

}

void analyze()
{
//same general procedure at the mutate function, except assinging a different function for the threads

pthread_create(&worker1, NULL, analyze_protege, &w1);
pthread_create(&worker2, NULL, analyze_protege, &w2);
pthread_create(&worker3, NULL, analyze_protege, &w3);
pthread_create(&worker4, NULL, analyze_protege, &w4);


pthread_join(worker1, NULL);
pthread_join(worker2, NULL);
pthread_join(worker3, NULL);
pthread_join(worker4, NULL);


}

void *analyze_protege(void *arg)
{

    //calculates the variance for all the sums in the squares, then assigns
    //the variance to the fitness member of the square structure

    //this function, like the mutate_protege function, is run in parrel, with each thread
    //performing this function on a portion of the population

    args *w = (args*)arg;

    int start = w->start;
    int finish = w->finish;

    int x, i, j, z;

    int *data = malloc(sizeof(int)*((size*2)+2));

    for(x = start; x < finish; x++)
    {

    int row_sum = 0;
    int column_sum = 0;

    for(i = 0; i < size; i++)
    {
        data[i] = 0;

        for(j = 0; j < size; j++)
        {
            data[i]+= MATRIX(x)[i][j];
        }

    }

    for(z = 0; z < size; z++)
    {
        data[i] = 0;

        for(j = 0; j < size; j++)
        {
            data[i]+= MATRIX(x)[j][z];

        }
        i++;
    }

    data[i] = 0;

    for(j = 0; j < size; j++)
    {
        data[i]+= MATRIX(x)[j][j];
    }

    i++;
    data[i] = 0;

    for(j = 0; j < size; j++)
    {
        data[i]+= MATRIX(x)[(size-1)-j][j];
    }

    int total = 0;

     for(i = 0; i < (size*2+2); i++)
      {
          total = total + data[i];
      }

    double average = (double)total/(double)(size*2+2);
    double variance = 0;

     for(i = 0; i < (size*2+2); i++)
      {
        //double dif = data[i]-average;
        double dif = data[i]-average;
        variance += dif*dif;
      }

    population[x].fitness = variance;

    }

    free(data);

    return 0;
}

void log_file(m_square square)
{

    //this function outputs a magic square to a text file

    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];
    char buffer_temp [80];
    int n;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    //this uses time info for

    strftime (buffer_temp,80,"%b%d_%I~%M%p.txt", timeinfo);
    //a very powerful function(if somewhat non-user friendly)
    //for creating strings out of c's time-info structure

    n = sprintf(buffer,"Magic_%dby%d_%s", size, size, buffer_temp);
    //sprintf is actually one of my favorite c functions, it's used
    //like printf, except it writes the output to a charbuffer
    //instead of standard out

    printf("\nThe magic square has been written to ");
    puts (buffer);

    FILE* output;
    output = fopen(buffer, "w");

    int i;
    int j;

    fprintf(output, "%d by %d magic square,\n", size, size);
    strftime (buffer,80,"Evolved on %b%d at %I:%M%p", timeinfo);
    fprintf(output, "%s", buffer);
    fprintf(output, "\n\n");

    for(j = 0; j < size; j++)
    {

        for(i = 0; i < size; i++)
        {
            fprintf(output, " %5d ",square.matrix[j][i]);
        }
        fprintf(output, "\n\n");
    }

    fclose(output);

    return;

}


void initialize()
{
    // Dispatches the threads, with their various argument-structures

    pthread_create(&worker1, NULL, initialize_protege, &w1);
    pthread_create(&worker2, NULL, initialize_protege, &w2);
    pthread_create(&worker3, NULL, initialize_protege, &w3);
    pthread_create(&worker4, NULL, initialize_protege, &w4);

    // Waits for the threads to finish
    pthread_join(worker1, NULL);
    pthread_join(worker2, NULL);
    pthread_join(worker3, NULL);
    pthread_join(worker4, NULL);

    return;
}

void *initialize_protege(void *arg)
{

    /*Uses a Fisher–Yates shuffle to create a random, distinct numbers in the square
    Treats the square as an array.*/

    args *w = (args*)arg;

    int start = w->start;
    int finish = w->finish;

    int max = size*size;
    int i, j;
    int x, y, r;

    for(i = start; i < finish; i++)
    {
        population[i].fitness = -1;

        for(j = 0; j < max; j++) {
            MATRIX(i)[(int)floor(j/size)][j%size] = j+1;
        }

        for(j = max-1; j > 0; j--)
        {
            r = (rand_gen(w->seed)%(j+1));

            x = MATRIX(i)[(int)floor(r/size)][r%size];
            y = MATRIX(i)[(int)floor(j/size)][j%size];

            MATRIX(i)[(int)floor(r/size)][r%size] = y;
            MATRIX(i)[(int)floor(j/size)][j%size] = x;
        }
    }

    return 0;

}

void print_matrix(m_square square)
{
    int i;
    int j;
    printf("\n");
    for(j = 0; j < size; j++)
    {

        for(i = 0; i < size; i++)
        {
            printf(" %4d ",square.matrix[j][i]);
        }

        printf("\n\n");
    }

    printf("fitness is at %f\n", square.fitness);
    return;
}

int rand_gen(unsigned int *seed)
{
    //thread safe linear congruential generator

    unsigned int next = *seed;
    int result;

    next *= 1103515245;
    next += 12345;
    result = (unsigned int) (next / 65536) % 2048;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    *seed = next;

    return result;
}
