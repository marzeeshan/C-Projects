#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <cstdlib>
#include <cstdio>

using namespace std;

class CarInfo
{
public:	
	string licPlate;
	char direction;
	int time;
	bool traversed;
};

char getNextDirection(char currDir){
	char nextDir = currDir;
	switch(currDir){
		case 'N': nextDir = 'E'; break;
		case 'E': nextDir = 'S'; break;
		case 'S': nextDir = 'W'; break;
		case 'W': nextDir = 'N'; break;
	}
	return nextDir;
}

string getDirectionName(char currDir){
	string name = "";
	switch(currDir){
		case 'E': name = "Eastbound"; break;
		case 'S': name = "Southbound"; break;
		case 'W': name = "Westbound"; break;
		case 'N': name = "Northbound"; break;
	}
	return name;
}

/*
	This method is called by each child process.
*/
void displayInfoByChildProcess(CarInfo carInfo){
	cout << "Current direction: " << getDirectionName(carInfo.direction) << endl;
	cout << "Car " << carInfo.licPlate << " is using the intersection for " << carInfo.time <<" sec(s)" << endl;
	// sleeps for the specified amount of time
	sleep(carInfo.time);
	// Then exits
	exit(0);
}

int main(int argc, char const *argv[])
{
	char currDir;
	int maxNumCar;

	// Reading current direction
	cin >> currDir;

	// Reading maximum number of cars per intersection
	cin >> maxNumCar;

	// Creating variables to read licence plate, direction and time to sleep.
	string lic;
	char dir;
	int time;

	// List containing car information
	vector<CarInfo> lstCars; 

	// Reading input file until it has valid formatted lines (each line having car information)
	while((cin >> lic >> dir >> time) != NULL){
		CarInfo car;
		car.licPlate = lic;
		car.direction = dir;
		car.time = time;
		car.traversed = false;
		lstCars.push_back(car);
	}

	// Creating order of car to be executed
    vector<CarInfo> orderedList;

    int index = 0;
    // Looping through each car information, based on current direction
    // and car information, re-arranging cars to put them in order in orderedList.
    while(index < lstCars.size()) {
    	for(int k=0; k<maxNumCar; k++){
    		for (int i = 0; i < lstCars.size(); i++) {
    			if(lstCars.at(i).direction == currDir && lstCars.at(i).traversed == false){
    				lstCars.at(i).traversed = true;
        			orderedList.push_back(lstCars.at(i));
        			index++;
        			break;
        		}
    		}
    	}
    	currDir = getNextDirection(currDir);
	}

	// Printing order information
	cout << "Direction\tLicense Plate\tTime\n"; 
	for (int i = 0; i < orderedList.size(); i++) {
        cout << orderedList.at(i).direction << "\t\t" << orderedList.at(i).licPlate << "\t\t" << orderedList.at(i).time << "\n"; 
	}

	// Creating child process for each car
	pid_t p_id;
	for (int i = 0; i < orderedList.size(); i++) {
		p_id = fork();
		if(p_id == 0){ 
			// Calling display method to display information about this car.
        	displayInfoByChildProcess(orderedList.at(i)); 
    	}
	}

	for (int i = 0; i < orderedList.size(); i++) {
		wait(NULL);
	}

	return 0;
}