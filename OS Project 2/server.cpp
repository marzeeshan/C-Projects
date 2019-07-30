
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/types.h>   
#include <sys/ipc.h>     
#include <sys/shm.h>  /* semaphore functions and structs.     */
#include <sys/sem.h>  /* shared memory functions and structs. */
#include <unistd.h> 
#include <sys/wait.h>

#include  <signal.h>

using namespace std;

#define MAX_LEN     64 /* length of string */
#define SEM_ID      250  /* ID for the semaphore.               */
#define BUFF_SIZE   100 // buffer size

struct account {
    int number;
    double balance;
    char name[MAX_LEN];
};

// golobal variable

char* shm_addr; // address of shared memory segment
int shm_id; // ID of the shared memory segment   
struct shmid_ds shm_desc;

void INThandler(int); /* Ctrl-C handler           */

void error(string msg) {
    cout << msg << "\n";
}

void sem_lock(int sem_set_id);
void sem_unlock(int sem_set_id);

void readFile(vector<account> & accs, char * file_name);

void putAccountOnSharedMemory(account* saccs, vector<account> & accs);

account * getAcc(int sem_set_id, int* acc_num, account* saccs, int request_num);

void childProcess(int sem_set_id, int* acc_num, account* saccs, int newsockfd);

void runServer(int portno, int sem_set_id, int* acc_num, account* saccs);

int main(int argc, char** argv) {

    char file_name[512] = "input.txt";
    int port_num = 123456;

    cout << "Enter path to input file: ";
    cin >> file_name;

    cout << "Enter port number: ";
    cin >> port_num;

    vector<account> accs;

    readFile(accs, file_name);
    
    
    //-------------------------------------------------------------------------
    // create shared memory 
    int sem_set_id; // ID of the semaphore set.          

    union semun { // semaphore value, for semctl().    
        int val;
        struct semid_ds *buf;
        ushort * array;
    } sem_val;


    int* acc_num; // number of accounts in shared mem
    account * saccs; // accounts array in shared mem

    int rc; // return value of system calls
    key_t semkey;

    /* Get unique key for semaphore. */
    if ((semkey = ftok("/tmp", 'a')) == (key_t) - 1) {
        perror("IPC error: ftok");
        exit(1);
    }

    // create semaphore set
    sem_set_id = semget(SEM_ID, 1, IPC_CREAT | 0600);
    if (sem_set_id == -1) {
        perror("creat: semget");
        exit(1);
    }

    // initialize the semaphore to '1'
    sem_val.val = 1;
    rc = semctl(sem_set_id, 0, SETVAL, sem_val);
    if (rc == -1) {
        perror("initialize: semctl");
        exit(1);
    }

    // allocate a shared memory segment with size of account array
    shm_id = shmget(IPC_PRIVATE, sizeof (account) * accs.size() + 128,
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

    // create a account index on the shared memory segment
    acc_num = (int*) shm_addr;
    *acc_num = 0;

    // skip memory space of the number of account
    saccs = (account*) (((void*) shm_addr) + sizeof (int));

    *acc_num = accs.size();
    putAccountOnSharedMemory(saccs, accs);

    //-------------------------------------------------------------------------
    signal(SIGINT, INThandler); /* install Ctrl-C handler   */

    //-------------------------------------------------------------------------
    runServer(port_num, sem_set_id, acc_num, saccs);




    return 0;
}

void runServer(int portno, int sem_set_id, int* acc_num, account* saccs) {
    int sockfd, clilen;

    struct sockaddr_in serv_addr, cli_addr;
    int n;

    //-------------------------------------------------------------------------
    // open socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof (serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof (cli_addr);


    //-------------------------------------------------------------------------
    // run the server

    while (true) {

        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *) & clilen);
        if (newsockfd < 0) {
            cout << "ERROR on accept";
            continue;
        }

        if (fork() == 0) {
            childProcess(sem_set_id, acc_num, saccs, newsockfd);
            exit(0);
        }

    }

    int child_status;

    wait(&child_status);

}

/**
 * fulfill request in child procee
 * @param sem_set_id
 * @param acc_num
 * @param saccs
 * @param newsockfd
 */
void childProcess(int sem_set_id, int* acc_num, account* saccs, int newsockfd) {
    char buffer[BUFF_SIZE];
    int request_type;
    int request_num;
    double request_val;
    int n;
    account * acc;
    double old_balance;
    bool connecting = true;
    int item;
    
    // continue to run until get ending message
    while (connecting == true) {
        
        // get message
        bzero(buffer, BUFF_SIZE);
        n = read(newsockfd, buffer, BUFF_SIZE - 1);
        buffer[n] = '\0';
        
        // parse it 
        item = sscanf(buffer, "%d %d %lf", &request_type, &request_num, &request_val);

        printf("Request message: %s", buffer);
        bzero(buffer, BUFF_SIZE);
        if (item == 3 || item == 1) {
            
            if (request_type != 4) {
                
                // get the selected account 
                acc = getAcc(sem_set_id, acc_num, saccs, request_num);
                if (acc != NULL) {
                    switch (request_type) {
                        case 0:
                            sprintf(buffer, "SUCCESS");
                            break;
                        case 1:
                            sem_lock(sem_set_id);
                            sprintf(buffer, "SUCCESS %f %f", acc->balance, acc->balance);
                            sem_unlock(sem_set_id);
                            break;
                        case 2:
                            sem_lock(sem_set_id);
                            old_balance = acc->balance;
                            acc->balance = old_balance + request_val;
                            sprintf(buffer, "SUCCESS %f %f", old_balance, acc->balance);
                            sem_unlock(sem_set_id);
                            break;
                        case 3:
                            sem_lock(sem_set_id);
                            old_balance = acc->balance;
                            acc->balance = old_balance - request_val;
                            sprintf(buffer, "SUCCESS %f %f", old_balance, acc->balance);
                            sem_unlock(sem_set_id);
                            break;
                        case 4:
                            connecting = false;
                            sprintf(buffer, "SUCCESS");
                            break;
                        default:
                            connecting = false;
                            sprintf(buffer, "FAILED");
                            break;
                    }
                } else {
                    connecting = false;
                    sprintf(buffer, "FAILED");
                }
            } else { // request_type == 4
                connecting = false;
                sprintf(buffer, "SUCCESS");
            }
        } else {
            connecting = false;
            sprintf(buffer, "FAILED");
        }

        printf("Response message: %s\n", buffer);
        n = write(newsockfd, buffer, strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket");
    }


}

/**
 * get accoutn from shared memory
 * @param sem_set_id
 * @param acc_num
 * @param saccs
 * @param request_num
 * @return 
 */
account * getAcc(int sem_set_id, int* acc_num, account* saccs, int request_num) {
    account * acc = NULL;
    sem_lock(sem_set_id);
    for (int i = 0; i < (*acc_num); i++) {
        if (saccs[i].number == request_num) {
            acc = saccs + i;
            break;
        }
    }
    sem_unlock(sem_set_id);
    return acc;
}

/**
 * put all account on the shared memory space
 * @param saccs
 * @param accs
 */
void putAccountOnSharedMemory(account* saccs, vector<account> & accs) {
    for (int i = 0; i < accs.size(); i++) {
        strcpy(saccs[i].name, accs[i].name);
        saccs[i].balance = accs[i].balance;
        saccs[i].number = accs[i].number;
    }
}

/**
 * get input file 
 * @param accs
 * @param file_name
 */
void readFile(vector<account> & accs, char * file_name) {
    int number;
    double balance;
    char name[MAX_LEN];

    ifstream in;
    in.open(file_name, ios::in);
    if (in.is_open()) {
        account acc;

        while (in >> acc.number >> acc.balance >> acc.name) {
            accs.push_back(acc);
        }

        in.close();
    }
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

/**
 * handle keyboard signal 
 * @param sig
 */
void INThandler(int sig) {
    char c;
    signal(sig, SIG_IGN); /* disable Ctrl-C           */
    printf("Did you hit Ctrl-C?\n" /* print something     */
            "Do you really want to quit? [y/n] ");
    c = getchar(); /* read an input character  */
    if (c == 'y' || c == 'Y') /* if it is y or Y, then    */ {
        // detach the shared memory segment 
        if (shmdt(shm_addr) == -1) {
            perror("detach: shmdt: ");
        }

        // de-allocate the shared memory segment
        if (shmctl(shm_id, IPC_RMID, &shm_desc) == -1) {
            perror("de-allocate: shmctl: ");
        }
        exit(0); /* exit.  Otherwise,        */
    } else {
        signal(SIGINT, INThandler); /* reinstall the handler    */
    }
}
