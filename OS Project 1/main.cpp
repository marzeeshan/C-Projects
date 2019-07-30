#include <cstdlib>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <iomanip>

using namespace std;

struct Matrix {
    int row;
    int col;
    double **m;
};

struct Data {
    Matrix * m1;
    Matrix * m2;
    int row;
    int col;
    double result;
    clock_t runtime;
};

Matrix * readMatrix(fstream & input);

void * adding(void * data);
void add(Matrix * m1, Matrix * m2);

void * subtracting(void * data);
void subtract(Matrix * m1, Matrix * m2);

void * multipling(void * data);
void multiple(Matrix * m1, Matrix * m2);

/*
 * 
 */
int main(int argc, char** argv) {

    fstream input;
    input.open("input.txt", ios::in);

    if (input.is_open() == false) {
        cout << "Cannot open file\n";
        return 0;
    }
    static Matrix * m1 = readMatrix(input);
    static Matrix * m2 = readMatrix(input);

    // close input stream
    input.close();

    int selection;

    // looping
    do {
        cout << "Enter your selection 1-4: ";
        cin >> selection;
        switch (selection) {
            case 1:
                add(m1, m2);
                break;
            case 2:
                subtract(m1, m2);
                break;
            case 3:
                multiple(m1, m2);
                break;
            case 4:
                break;
            default:
                cout << "non-valid option\n";
        }

    } while (selection != 4);



    return 0;
}

void multiple(Matrix * m1, Matrix * m2) {
    
    // check for valid matrices 
    if (m1->col != m2->row
            || m1->col <= 0
            || m1->row <= 0) {
        cout << "Their dimension is not matched \n";
        return;
    }

    // create array of thread
    pthread_t tid[m1->row][m2->col];
    // create array of thread data 
    Data data[m1->row][m2->col];

    // create threads
    for (int i = 0; i < m1->row; i++) {
        for (int j = 0; j < m2->col; j++) {

            data[i][j].col = j;
            data[i][j].row = i;
            data[i][j].m1 = m1;
            data[i][j].m2 = m2;

            int result = pthread_create(&tid[i][j], NULL, multipling, (void *) &(data[i][j]));
            if (result) {
                return;
            }
        }
    }

    // join thread 
    clock_t runtime = 0;
    for (int i = 0; i < m1->row; i++) {
        for (int j = 0; j < m1->col; j++) {
            pthread_join(tid[i][j], NULL);
            // show result
            cout << setw(4) << data[i][j].result;
            runtime += data[i][j].runtime;
        }
        cout << "\n";
    }

    // show run time 
    cout << "Average run time : "
            << (double) runtime / (double) (m1->row * m1->col)
            << " milliseconds\n";
};

void * multipling(void * data) {
    Data * dt = (Data *) data;
    
    // set start time 
    dt->runtime = clock();
    dt->result = 0;
    
    // pointer to right matrix 
    Matrix * m1 = dt->m1;
    Matrix * m2 = dt->m2;
    
    // get result
    for (int i = 0; i < m1->col; i++) {
        dt->result += m1->m[dt->row][i] * m2->m[i][dt->col];
    }    
    
    // set run time 
    dt->runtime = clock() - dt->runtime;
    pthread_exit((void*) 0);
};

void subtract(Matrix * m1, Matrix * m2) {
    
    // check for valid matrices 
    if (m1->row != m2->row
            || m1->col != m2->col
            || m1->col <= 0
            || m1->row <= 0) {
        cout << "Their dimension is not matched \n";
        return;
    }

    // create array of thread
    pthread_t tid[m1->row][m1->col];
    // create array of thread data 
    Data data[m1->row][m1->col];



    for (int i = 0; i < m1->row; i++) {
        for (int j = 0; j < m1->col; j++) {

            data[i][j].col = j;
            data[i][j].row = i;
            data[i][j].m1 = m1;
            data[i][j].m2 = m2;

            int result = pthread_create(&tid[i][j], NULL, subtracting, (void *) &(data[i][j]));
            if (result) {
                return;
            }
        }
    }

    // join thread 
    clock_t runtime = 0;
    for (int i = 0; i < m1->row; i++) {
        for (int j = 0; j < m1->col; j++) {
            pthread_join(tid[i][j], NULL);
             // show result
            cout << setw(4) << data[i][j].result;
            runtime += data[i][j].runtime;
        }
        cout << "\n";
    }

     // show run time 
    cout << "Average run time : "
            << (double) runtime / (double) (m1->row * m1->col)
            << " milliseconds\n";

    // pthread_exit(NULL);
}

void * subtracting(void * data) {
    Data * dt = (Data *) data;
    // set start time 
    dt->runtime = clock();
    
    dt->result = dt->m1->m[dt->row][dt->col] - dt->m2->m[dt->row][dt->col];
    
    // set run time 
    dt->runtime = clock() - dt->runtime;
    
    // exit thread
    pthread_exit((void*) 0);
}

void add(Matrix * m1, Matrix * m2) {
    
    // check for valid matrices 
    if (m1->row != m2->row
            || m1->col != m2->col
            || m1->col <= 0
            || m1->row <= 0) {
        cout << "Their dimension is not matched \n";
        return;
    }

    // create array of thread
    pthread_t tid[m1->row][m1->col];
    // create array of thread data 
    Data data[m1->row][m1->col];


    // create threads
    for (int i = 0; i < m1->row; i++) {
        for (int j = 0; j < m1->col; j++) {

            data[i][j].col = j;
            data[i][j].row = i;
            data[i][j].m1 = m1;
            data[i][j].m2 = m2;

            int result = pthread_create(&tid[i][j], NULL, adding, (void *) &(data[i][j]));
            if (result) {
                return;
            }
        }
    }

    // join thread 
    clock_t runtime = 0;
    for (int i = 0; i < m1->row; i++) {
        for (int j = 0; j < m1->col; j++) {
            pthread_join(tid[i][j], NULL);
            // show result
            cout << setw(4) << data[i][j].result;
            runtime += data[i][j].runtime;
        }
        cout << "\n";
    }

    // show run time 
    cout << "Average run time : "
            << (double) runtime / (double) (m1->row * m1->col)
            << " milliseconds\n";

}

void * adding(void * data) {
    Data * dt = (Data *) data;
    // set start time 
    dt->runtime = clock();
    
    dt->result = dt->m1->m[dt->row][dt->col] + dt->m2->m[dt->row][dt->col];
    
    // set run time 
    dt->runtime = clock() - dt->runtime;
    // exit thread
    pthread_exit((void*) 0);
}

Matrix * readMatrix(fstream & input) {

    // allocate new memory form heap
    Matrix * m = new Matrix;
    
    // get row and column 
    input >> m->row >> m->col;
    
    // allocate new memory form heap
    m->m = new double*[m->row];
    
    // read data 
    for (int i = 0; i < m->row; i++) {
        // allocate new memory form heap
        m->m[i] = new double[m->col];
        for (int j = 0; j < m->col; j++) {
            input >> m->m[i][j];
        }

    }

    return m;
}
