#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <queue> 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


using namespace std;

class DatabaseRequest
{
public:	
	int group;
	int id;
	int position;
	int time;
	int duration;
};

int startingGroup;
int totalFromStartingGroup = 0;
int finishedStartingGroup = 0;

vector<pthread_cond_t> conds; 
vector<pthread_cond_t> condsPos; 
queue<int> waitingIDs;
queue<int> waitingPos;
int positions[10]; 
int userPos[10]; 
queue<int> waitPos[10];
int printed = 0;

int group1Count = 0;
int group2Count = 0;
int waitDueToGroup = 0;
int waitDueToPosition = 0;

//pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 
 
vector<pthread_mutex_t> locks; 
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock5 = PTHREAD_MUTEX_INITIALIZER;

int waitIndex = 0;

void update(int a) {
	pthread_mutex_lock(&lock2);
	totalFromStartingGroup += a;
	pthread_mutex_unlock(&lock2); 
}

void updateGroupWait() {
	pthread_mutex_lock(&lock5);
	waitDueToGroup += 1;
	pthread_mutex_unlock(&lock5); 
}

void updatePositionWait() {
	pthread_mutex_lock(&lock5);
	waitDueToPosition += 1;
	pthread_mutex_unlock(&lock5); 
}

void addToWait(int a) {
	pthread_mutex_lock(&lock3);
	waitingIDs.push(a);
	pthread_mutex_unlock(&lock3); 
}

void updatePosition(int pos, int value, int id) {
	pthread_mutex_lock(&lock4);
	positions[pos] += value;
	userPos[pos] = id;
	pthread_mutex_unlock(&lock4); 
}

void addToWaitPos(int pos, int a) {
	pthread_mutex_lock(&lock3);
	waitPos[pos].push(a);
	pthread_mutex_unlock(&lock3); 
}

void *execute(void *args) {

	DatabaseRequest *req = (DatabaseRequest *) args;

	printf("User %d from Group %d arrives to the DBMS\n", req->id, req->group);

	//
	if(totalFromStartingGroup > 0 && req->group != startingGroup){
		printf("User %d is waiting due to its group\n", req->id);
		addToWait(req->id-1);
		updateGroupWait();
		pthread_mutex_lock(&lock1);
		pthread_cond_wait(&(conds.at(req->id-1)), &lock1); 
		pthread_mutex_unlock(&lock1);
	}

	if(positions[req->position-1] != 0) {
		printf("User %d is waiting: position %d of the database is being used by user %d\n", 
			req->id, req->position, userPos[req->position-1]);
		addToWaitPos(req->position-1, req->id-1);
		updatePositionWait();
		pthread_mutex_lock(&lock1);
		pthread_cond_wait(&(condsPos.at(req->id-1)), &lock1); 
		pthread_mutex_unlock(&lock1);
	}

	updatePosition(req->position-1, 1, req->id);

	printf("User %d is accessing the position %d of the database for %d second(s)\n", 
		req->id, req->position, req->duration);

	sleep(req->duration);

	printf("User %d finished its execution\n", req->id);

	//printf("1. Waiting Position %d\n", positions[req->position-1]);
	updatePosition(req->position-1, -1, 0);
	if(positions[req->position-1] == 0 && waitPos[req->position-1].size() > 0){
		int p = waitPos[req->position-1].front();
		//printf("Postion is %d\n", p);
		pthread_cond_signal(&(condsPos.at(p)));
		waitPos[req->position-1].pop();
	}

	if(req->group == startingGroup) {
		update(-1);
	}
	
	//pthread_mutex_lock(&(locks.at(req->id-1)));
	if(totalFromStartingGroup <= 0 ) {
		//printf("Waiting Size: %lu\n", waitingIDs.size());
		//pthread_mutex_lock(&lock1);
		if(printed == 0) {
			printed = 1;
			printf("\n%s\n", "All users from Group 1 finished their execution");
			printf("%s\n\n", "The users from Group 2 start their execution");
		}

		if(waitingIDs.size() > 0) {
			//printf("Waiting request is %d\n", waitingIDs.front()+1);
			pthread_cond_signal(&(conds.at(waitingIDs.front())));
			waitingIDs.pop();
		}
		//pthread_mutex_unlock(&lock1); 
	}	
	//

	//printf("%d\n", );

    return NULL;
}


int main(int argc, char const *argv[]) {

	// Reading starting group
	cin >> startingGroup;

	int group;

	int position;

	int time;

	int duration;

	// List containing requests information
	vector<DatabaseRequest> lstRequests; 

	int pos = 1;

	// Reading input file until it has valid formatted lines (each line having car information)
	while((cin >> group >> position >> time >> duration) != NULL){
		DatabaseRequest req;
		req.group = group;
		req.id = pos++;
		req.position = position;
		req.time = time;
		req.duration = duration;
		lstRequests.push_back(req);

		if(req.group == 1) {
			group1Count++;
		}else {
			group2Count++;
		}

		pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 
		conds.push_back(cond);


		pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER; 
		condsPos.push_back(cond2);

		pthread_mutex_t l = PTHREAD_MUTEX_INITIALIZER; 

		//pthread_mutex_init(&l, NULL);
		locks.push_back(l);

		if(req.group == startingGroup) {
			totalFromStartingGroup++;
		}
	}

	for(int i=0; i<10; i++) {
		positions[i] = 0;
		userPos[i] = 0;
	}

	pthread_t threads[lstRequests.size()];

	int rc;
	pthread_mutex_init(&lock1, NULL);
	pthread_mutex_init(&lock2, NULL);
	pthread_mutex_init(&lock3, NULL);
	pthread_mutex_init(&lock4, NULL);

	// Printing request information 
	for (int i = 0; i < lstRequests.size(); i++) {
        rc = pthread_create(&threads[i], NULL, execute, (void *)&lstRequests.at(i));
	    if (rc) {
	      cout << "Error:unable to create thread," << rc << endl;
          exit(-1);
	    }
	}

	void *status;
	for(int i = 0; i < lstRequests.size(); i++ ) {
      rc = pthread_join(threads[i], &status);
      if (rc) {
         cout << "Error:unable to join," << rc << endl;
         exit(-1);
      }
    }

    printf("\n%s\n", "Total Requests:");
    printf("\t\tGroup 1: %d\n", group1Count);
    printf("\t\tGroup 2: %d\n", group2Count);

    printf("\n%s\n", "Requests that waited:");
    printf("\tDue to its group: %d\n", waitDueToGroup);
    printf("\tDue to a locked position: %d\n", waitDueToPosition);

    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
    pthread_mutex_destroy(&lock3);
    pthread_mutex_destroy(&lock4);
	pthread_exit(NULL);

	return 0;
}