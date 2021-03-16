//inport Libarary
#include <chrono>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "md5wrapper.h"



//Declaring functions for clock
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::cout;
using std::cin;
using std::string;
using std::ifstream;
using std::vector;
using std::mutex;
using std::condition_variable;


//Define clock
typedef std::chrono::steady_clock theClock;


//Declaring global variables
std::atomic<bool> work;
mutex thread_safe;
condition_variable cv;



//Function to compare the inputed hash to the dictionary using 1 thread
void singleCrack(string hash, vector<string>& wordList) {//Crack using 1 thread

	md5wrapper md5;

	for (unsigned i = 0; i < wordList.size(); i++) {//Loop through dictionary

		if (md5.getHashFromString(wordList.at(i)) == hash) {//Compare hashes
			cout << "\n The result is: " << wordList.at(i) << std::endl << std::endl;//output
			return;
		}

	}

}


//Crack threaded and automatically splits the dictionary
void threadCrack(string& hash, vector<string>& wordList) {

	md5wrapper md5;

	//Loop through dictionary
	for (unsigned i = 0; i < wordList.size(); i++) {

		//Check to see if hashes match
		if (md5.getHashFromString(wordList.at(i)) == hash) {

			std::unique_lock<mutex> lock(thread_safe);
			cout << "\n\nThe result is: " << wordList.at(i) << std::endl << std::endl;
			//Make other threads return if correct
			work = true;
			
			cv.notify_all();

		}
		else if (work == true) {

			return;//Return if any other thread found result

		}

	}

}



//Main Cracking function for multi thread, if 1 thread is chosen then crack function runs
void runCracker(string& hash, int& numThreads) {


	string line;

	vector<std::thread> thread(numThreads);//Allocating max memory
	vector<string> wordList;
	vector<vector<string>> splitList(numThreads, vector<string>(370103 / numThreads));//Allocating max memory of 2D dictionary vector


	//Read in full dictionary
	ifstream d("words_alpha.txt");
	if (d.is_open()) {

		while (getline(d, line)) {
			wordList.push_back(line);
		}

		d.close();
	}

	cout << "\nLoaded Word List Successfully" << std::endl << std::endl;//output


	//Checking if the user wants to use more than 1 frame
	if (numThreads > 1) {

		//Split dictionary between threads
		for (int i = 0; i < numThreads; i++) {

			for (int j = 0; j < (370103 / numThreads); j++) {

				splitList.at(i).at(j) = wordList.at(j + (i * (370103 / numThreads)));

			}

		}

		cout << "\nWord List split into threads" << std::endl << std::endl;//output

	}




	//Checking to see which function to run based on the users thread choice
	if (numThreads == 1) {

		theClock::time_point start = theClock::now();//Starting timer
		singleCrack(hash, wordList);//Run crack() function
		theClock::time_point end = theClock::now(); //End timing
		auto time_taken = duration_cast<milliseconds>(end - start).count();//Calculate the time taken

		cout << "\nThe program took " << time_taken << " milliseconds" << std::endl << std::endl;//output

	}
	else {

		theClock::time_point start = theClock::now(); // Start timing

		for (int i = 0; i < numThreads; i++) {

			thread[i] = std::thread(threadCrack, std::ref(hash), std::ref(splitList[i]));//Run the threadCrack() with the multi threads
			std::unique_lock<mutex> lock(thread_safe);

			while (work) {

				cv.wait(lock);
				if (!work)
					cout << "Spurious wake up!\n";

			}

		}

		for (int i = 0; i < numThreads; i++) {

			thread[i].join();//rejoin threads to main() function

		}

		theClock::time_point end = theClock::now();//End timing
		auto time_taken = duration_cast<milliseconds>(end - start).count();//Calculate time taken
		cout << "\nIt took " << time_taken << " milliseconds..." << std::endl << std::endl;//output


	}

}

int main() {

	//Declaring variables to pass to functions
	bool run = true;
	string hash;
	int numThreads;

	//Main program loop
	while (run) {

		//Resets at the start of each loop
		numThreads = NULL;
		work = false;
		std::thread thread;

		cout << "Enter a MD5 hash to be cracked ";//output
		cin >> hash;

		cout << "Enter how many threads are to be used ";//output
		cin >> numThreads;

		cout << "\nCracking" << std::endl;//output

		//Runs main cracking function
		runCracker(hash, numThreads);

		system("pause");
		system("cls");

	}

	return 0;

}