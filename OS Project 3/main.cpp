#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <cstdio>

#include <sys/types.h>   
#include <sys/ipc.h>     
#include <sys/shm.h>  /* semaphore functions and structs.     */
#include <sys/sem.h>  /* shared memory functions and structs. */
#include <unistd.h> 
#include <sys/wait.h>
#include <stdbool.h>


using namespace std;

#define MAX_LEN     64 /* length of string */
#define SEM_ID      250  /* ID for the semaphore.               */

struct client {
    char name[MAX_LEN];
    int arriving; // number of second elapsed since the arrived of the previous client
    int serving; // number of second will be processed by the clerk
    bool waiting;
};



// golobal variable

char* shm_addr; // address of shared memory segment
int shm_id; // ID of the shared memory segment   
struct shmid_ds shm_desc;


void error(string msg) {
    cout << msg << "\n";
}


void getInput(vector<client> & clients);
int createSharedMemory(int memory_size);
void deallocateShareMemory();

void putClientToSharedMemory(client * c_arr, vector<client> & clients);

void sem_lock(int sem_set_id);
void sem_unlock(int sem_set_id);
void clientProcess(int sem_set_id, int * clerk_num, client * c_arr, int index);

int main(int argc, char** argv) {

    if (argc < 2) {
        return 0;
    }

    int clerk_num = atoi(argv[1]);

    if (clerk_num <= 0 || clerk_num > 16) {
        return 0;
    }

    vector<client> clients;
    getInput(clients);

    int* clerk; // number of clerk in shared mem
    int * client_num;
    client * c_arr; // pointer to array of client in shared memory 

    // allocate a shared memory segment with size of an integer, 
    // the number of available clerk + size of another integer which 
    // is the number of client 
    // and client data
    int sem_set_id = createSharedMemory(sizeof (int) + sizeof (int)
            +clients.size() * sizeof (client) + 64);

    // point to the shared memory segment

    // clerk number
    clerk = (int*) shm_addr;
    *clerk = clerk_num;

    // client number
    client_num = clerk + 1;
    *client_num = clients.size();

    // client array
    c_arr = (client *) (clerk + 2);

    // put data to shared memory space 
    putClientToSharedMemory(c_arr, clients);

    /// fork all required processes 
    for (int i = 0; i < clients.size(); i++) {
        sleep(clients[i].arriving);
        if (fork() == 0) {
            clientProcess(sem_set_id, clerk, c_arr, i);
            exit(0);
        }
    }


    // wait for all children to stop 
    int child_status;

    for (int i = 0; i < clients.size(); i++) {
        wait(&child_status);
    }
    
    // count number of waiting client 
    int waiting_num = 0;
    
    for (int i = 0; i < *client_num; i++) {
        if (c_arr[i].waiting == true) {
            waiting_num++;
        }
    }

    deallocateShareMemory();
    
    // show the last message 
    cout << "\n";
    cout << "The total number of customers that got serviced: " 
            << clients.size() << "\n";
    cout << "Number of customers that didn't wait: " 
            << clients.size() - waiting_num << "\n";
    cout << "Number of customers that had to wait: "
            << waiting_num << "\n";
    
    return 0;
}

/**
 * process the task
 * @param sem_set_id id of the semaphore 
 * @param clerk_num pointer points to number of clerk in shared memory
 * @param c_arr pointer points to array of client  in shared memory
 * @param index index of the current client
 */
void clientProcess(int sem_set_id, int * clerk_num, client * c_arr, int index) {
    client * cl = c_arr + index;

    // print the first message 
    string name = string(cl->name);
    string msg = name + " arrives at the restaurant\n";
    cout << msg;
    cout.flush();

    bool waiting = true;
    int counter = 0;
    
    // loop until get change to get served 
    do {
        
        // look shared memory
        sem_lock(sem_set_id);        
        /// check for the number of available clerk
        if ((*clerk_num) > 0) {            
            *clerk_num = (*clerk_num) - 1;
            waiting = false;
        }
        // release the lock
        sem_unlock(sem_set_id);
        
        // there isn't any clerk
        if (waiting == true) {
            // sleep for 1 second
            sleep(1);
            counter++;
        }
    } while (waiting == true);

    msg = "\t" + name + " is getting helped\n";

    cout << msg;
    cout.flush();

    // serving time 
    sleep(cl->serving);


    // update the number of available  clerk
    sem_lock(sem_set_id);
    
    *clerk_num = (*clerk_num) + 1;
    
    if (counter == 0) {
        c_arr[index].waiting = false;
    } else {
        c_arr[index].waiting = true;
    }
    
    sem_unlock(sem_set_id);
    
    // print message to screen 
    msg = "\t\t" + name + " leaves the restaurant\n";

    cout << msg;
    cout.flush();
}

/**
 * lock shared memory
 * @param sem_set_id
 */
void sem_lock(int sem_set_id) {
    struct sembuf sem_op;

    sem_op.sem_num = 0;
    sem_op.sem_op = -1; // lock it
    sem_op.sem_flg = 0;
    semop(sem_set_id, &sem_op, 1);
}

/**
 * unlock shared memory
 * @param sem_set_id
 */
void sem_unlock(int sem_set_id) {
    struct sembuf sem_op;

    sem_op.sem_num = 0;
    sem_op.sem_op = 1; // release it
    sem_op.sem_flg = 0;
    semop(sem_set_id, &sem_op, 1);
}

void putClientToSharedMemory(client * c_arr, vector<client> & clients) {
    // copy item of vector to shared memory
    for (int i = 0; i < clients.size(); i++) {
        strcpy(c_arr[i].name, clients[i].name);
        c_arr[i].arriving = clients[i].arriving;
        c_arr[i].serving = clients[i].serving;
        c_arr[i].waiting = false;
    }
}

void deallocateShareMemory() {
    // detach the shared memory segment 
    if (shmdt(shm_addr) == -1) {
        perror("detach: shmdt: ");
    }

    // de-allocate the shared memory segment
    if (shmctl(shm_id, IPC_RMID, &shm_desc) == -1) {
        perror("de-allocate: shmctl: ");
    }
}

int createSharedMemory(int memory_size) {
    //-------------------------------------------------------------------------
    // create shared memory 
    int sem_set_id; // ID of the semaphore set.          

    union semun { // semaphore value, for semctl().    
        int val;
        struct semid_ds *buf;
        ushort * array;
    } sem_val;


    // create semaphore set
    sem_set_id = semget(SEM_ID, 1, IPC_CREAT | 0600);
    if (sem_set_id == -1) {
        perror("creat: semget");
        exit(1);
    }


    // initialize the semaphore to '1'
    sem_val.val = 1;
    int rc = semctl(sem_set_id, 0, SETVAL, sem_val);
    if (rc == -1) {
        perror("initialize: semctl");
        exit(1);
    }

    // allocate a shared memory segment with size of an integer, 
    // the number of available clerk
    shm_id = shmget(IPC_PRIVATE, memory_size,
            IPC_CREAT | IPC_EXCL | 0600);
    if (shm_id == -1) {
        perror("allocate: shmget: ");
        exit(1);
    }

    // attach the shared memory segment to our process's address space. 
    shm_addr = (char *) shmat(shm_id, NULL, 0);
    if (!shm_addr) { /* operation failed. */
        perror("attach: shmat: ");
        exit(1);
    }

}

void getInput(vector<client> & clients) {
    client cl;
    clients.clear();
    string name;
    // read data from std input
    while (cin >> name >> cl.arriving >> cl.serving) {
        strcpy(cl.name, name.c_str());
        clients.push_back(cl);
    }
}