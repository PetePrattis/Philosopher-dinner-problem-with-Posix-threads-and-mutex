#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __unix__
# include <unistd.h>
#elif defined _WIN32
# include <windows.h>
#define sleep(x) Sleep(1000 * (x))
#endif

int philosophers;//count of philosophers

typedef struct philosopher_struct{//a struct that contains each philosopher's information
  int index;//each philosophers index
    pthread_t thread;//a POSIX thread for each philosopher
	pthread_mutex_t *left, *right;//the POSIX mutex for each philosopher
	int meal;
	double time_sum;//sum of time each philosopher was hungry
	int time_ready; //sum of times each philosopher was hungry
	int priority;
} philosopher;//the struct variable


int next(int i){//function to check the right fork of current philosopher
	int p = (i+1) % philosophers;
	if (p == 0)
        p = philosophers;

	return p;
}

int previous(int i){//function to check the left fork of current philosopher
	int p = (i-1) % philosophers;
	if (p == 0)
        p = philosophers;

	return p;
}

double ready_time_average(int i, int n, double t){//function that calculates the average time each philosopher was hungry
	printf("Philosopher %d was HUNGRY on average for %f seconds. \n", i, (t/n) );

	return t/n;
}

const char * Current_Time(){//function to return current timestamp
    time_t raw_time;
    raw_time = time(NULL);

    return ctime(&raw_time);
}

void *Philosopher_Meal(void *p){//function that executes the meal routine of each thread / philosopher
	philosopher *phil = (philosopher *)p;//a struct variable type of philosopher
	pthread_mutex_t *fork_left, *fork_right;//2 mutex variables for each philosopher
	int grab_left_fork, grab_right_fork;//2 variables to determine if effort to grab any fork was successful
	int time_full =0;
	time_t start_ready,end_ready;//variables that help calculate hungry time
	while (time_full <= 20){
		printf("Philosopher %d is THINKING at %s \n", phil->index, Current_Time());
		sleep(1+ rand() % 10);//random amount of time at thinking
		fork_left = phil->left;
		fork_right = phil->right;
		printf("Philosopher %d is HUNGRY at %s  \n", phil->index, Current_Time());
		start_ready = time(NULL);
		grab_left_fork = pthread_mutex_trylock(fork_left);//attempt to grab left fork
        if (grab_left_fork != 0){
            printf("Philosopher %d failed to take fork %d , because philosopher %d  was eating at %s \n",phil->index, phil->index, previous(phil->index), Current_Time());
            pthread_mutex_lock(fork_left);
        }
        grab_right_fork = pthread_mutex_trylock(fork_right);//attempt to grab right fork

        if (grab_right_fork != 0){
          printf("Philosopher %d failed to take fork %d , because philosopher %d was eating at %s \n",phil->index, next(phil->index), next(phil->index), Current_Time());
          pthread_mutex_lock(fork_right);
        }
        //-----critical section-----
        end_ready = time(NULL);//if both fork / mutex have been acquired philosopher starts eating
		phil->time_sum += difftime(end_ready, start_ready);
		phil->time_ready++;
		printf("Philosopher %d is EATING at %s  \n",phil->index, Current_Time());
		sleep(phil->priority);
		time_full += phil->index;
		pthread_mutex_unlock(fork_right);//unlock mutex
		pthread_mutex_unlock(fork_left);
		//--------------------------
 	}
	printf("Philosopher %d is FULL and LEAVES the table  at %s \n",phil->index, Current_Time());
	phil->meal = 1;

    return NULL;
}


void Philosophers_Problem(int N){//function that manages the threads and mutex for each philosopher

	pthread_mutex_t forks[N];//number of mutex
	philosopher phils[N];//number of philosopher structs
	philosopher *phil;//pointer for each struct
	int i;

	for (i =0; i< N ; i++){
		pthread_mutex_init(&forks[i], NULL);//initialize mutex / forks
	}
	for (i =0; i< N; i++){//initialize struct and create thread
		phil = &phils[i];
		phil->time_sum=0.0;
		phil->time_ready=0;
		phil->meal = 0;
		phil->index = i+1;
		phil->left = &forks[i];
		phil->right = &forks[(i+1)%N];
		phil->priority = rand()%3 +1;
		pthread_create(&phil->thread, NULL, Philosopher_Meal, phil);
	}
	int live = 1;
	while (live){//while threads are live
		int j=0;
		for (i=0; i<N; i++){
			phil = &phils[i];
			if(phil->meal){
                j +=1;
            }
		}
		if (j == N){//philosophers finished their meal
			for (i=0; i<N; i++){

				phil = &phils[i];
				pthread_join(phil->thread,NULL);
			}
			printf("All Philosophers are FULL now \n");
			double all_ready_time_average;
			for (i=0; i<N; i++){
				pthread_mutex_destroy(&forks[i]);
				phil = &phils[i];
				all_ready_time_average +=ready_time_average(phil->index, phil->time_ready, phil->time_sum);
			}
			printf("The average time of all Philosophers being HUNGRY is %f seconds. \n",(all_ready_time_average/N));
			live = 0;
		}
	}
}


int main()//main function
{
	philosophers = 0;
	while(philosophers > 10 || philosophers < 3){
		printf("Enter the number of Philosophers to dine! Number must be between 3 and 10: ");
		scanf("%d",&philosophers);
	}
	Philosophers_Problem(philosophers);

	return 0;
}
