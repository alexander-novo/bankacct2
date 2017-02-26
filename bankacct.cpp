/* -----------------------------------------------------------------------------

	FILE:              bankacct.cpp
	DESCRIPTION:       A bank account simulator which keeps track of bank accounts and lets you manage them
	COMPILER:          Built on g++ with c++11
	Exit Codes:
		- 0: All good
		- 1: Database file not defined
		- 2: Could not load Database file
		- 3: An account was needed, but not supplied
		- 4: Information was needed, but not supplied
		- 5: The report file could not be written
		- 6: An account to transfer to was needed, but not supplied
		- 7: The ammount of money to transfer was too much such that it would bring someone's balance negative
	
	MODIFICATION HISTORY:
	Author                  Date               Version
	---------------         ----------         --------------
	Alexander Novotny       10/18/2016         1.0.0
	Alexander Novotny       10/20/2016         2.0.0
----------------------------------------------------------------------------- */

#include <cstring>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm> //For std::sort
#include <regex>
#include <iomanip>
#include <iostream>
#include <map>
#include "bankacct.h"

using namespace std;

void sortArgs(map<char, vector<char*>>*, int, char*[]);
int parseArgs(map<char, vector<char*>>*, vector<Account>*);
char* yankArg(map<char, vector<char*>>*, char);

void helpMenu();
void displayInfo(Account*);

Account* findAccount(vector<Account>*, char*, char*);

bool createReport(vector<Account>*, char*);
bool loadDatabase(vector<Account>*, char*);

/* -----------------------------------------------------------------------------
FUNCTION:          main()
DESCRIPTION:       Sorts the arguments, then runs them
RETURNS:           See Exit Codes
----------------------------------------------------------------------------- */
int main(int argc, char* argv[]) {
	vector<Account> people;
	map<char, vector<char*>> args;

	sortArgs(&args, argc, argv);
	return parseArgs(&args, &people);	
}

/* -----------------------------------------------------------------------------
FUNCTION:          sortArgs()
DESCRIPTION:       Takes all of the raw command line arguments and sorts them into a map
RETURNS:           Void function
NOTES:             The map's keys are the switches,
                   while the values are a list of values supplied in the order in which they were supplied

                   So for example, for this command:
                   ./bankacct /Fblah /Hblah2 /Fblah3
                   The map would look like this:
                   [F] -> {blah, blah3}
                   [H] -> {blah2}
----------------------------------------------------------------------------- */
void sortArgs(map<char, vector<char*>>* args, int argc, char* argv[]) {
	char* arg;
	for(int i = 0; i < argc; i++) {
		arg = argv[i];
		//Check for a valid argument
		if(arg[0] != SLASH || arg[1] == '\0') continue;
		
		//Check if we already have one of these options
		//If not, create a new vector
		//Otherwise add to the existing one
		if(args->find(arg[1]) == args->end()) {
			vector<char*> vec;
			vec.push_back(arg + 2);
			args->insert(make_pair(arg[1], vec));
		} else {
			args->at(arg[1]).push_back(arg + 2);
		}
	}
}

/* -----------------------------------------------------------------------------
FUNCTION:          parseArgs()
DESCRIPTION:       Goes through the list of arguments and actually performs the functions
RETURNS:           See Exit Codes
----------------------------------------------------------------------------- */
int parseArgs(map<char, vector<char*>>* args, vector<Account>* people) {	
	//Conditions for help menu
	if(args->empty() || args->find(O_HELP) != args->end()) {
		helpMenu();
	}
	
	//If the database file hasn't been defined, quit
	if(args->find(O_DATA) == args->end()) {
		return ERR_NO_DB;
	}

	//Load the database file
	//If two databases are specified, default to the last one
	//If we weren't succesful, return
	if(!loadDatabase(people, args->at(O_DATA).back())) {
		cout << "ERR! Could not load \"" << args->at(O_DATA).back() << "\"";
		return ERR_DB_NOT_FOUND;
	}

	//WriteOnShutdown is a class which writes my database file whenever I exit, for any reason
	WriteOnShutdown write(args->at(O_DATA).back(), people);

	//Sort by Account number
	sort(people->begin(), people->end(), [](Account& a, Account& b) {
		return strcmp(a.number, b.number) < 0;
	});

	Account* acc;
	Account* acc2;
	char* buf;

	for(pair<char, vector<char*>> arg : *args) {
		//Ordered by priority
		switch(arg.first) {
			case O_CHANGE_AREA:
				acc = findAccount(people, yankArg(args, O_NUM), yankArg(args, O_PASS));
				if(acc == nullptr) {
					if(acc2 == nullptr) return ERR_NO_ACCOUNT;
					else acc = acc2;
				}
				buf = yankArg(args, O_CHANGE_AREA);
				if(buf == nullptr || !regex_match(buf, regex(R_AREA))) return ERR_NO_INFO;
				acc->area = atoi(buf);
				break;
			case O_CHANGE_F:
				acc = findAccount(people, yankArg(args, O_NUM), yankArg(args, O_PASS));
				if(acc == nullptr) {
					if(acc2 == nullptr) return ERR_NO_ACCOUNT;
					else acc = acc2;
				}
				buf = yankArg(args, O_CHANGE_F);
				if(buf == nullptr || !regex_match(buf, regex(R_NAME))) return ERR_NO_INFO;
				strcpy(acc->first, buf);
				break;
			case O_CHANGE_PHONE:
				acc = findAccount(people, yankArg(args, O_NUM), yankArg(args, O_PASS));
				if(acc == nullptr) {
					if(acc2 == nullptr) return ERR_NO_ACCOUNT;
					else acc = acc2;
				}
				buf = yankArg(args, O_CHANGE_PHONE);
				if(buf == nullptr || !regex_match(buf, regex(R_PHONE))) return ERR_NO_INFO;
				acc->phone = atoi(buf);
				break;	
			case O_CHANGE_L:
				acc = findAccount(people, yankArg(args, O_NUM), yankArg(args, O_PASS));
				if(acc == nullptr) {
					if(acc2 == nullptr) return ERR_NO_ACCOUNT;
					else acc = acc2;
				}
				buf = yankArg(args, O_CHANGE_L);
				if(buf == nullptr || !regex_match(buf, regex(R_NAME))) return ERR_NO_INFO;
				strcpy(acc->last, buf);
				break;
			case O_CHANGE_M:
				acc = findAccount(people, yankArg(args, O_NUM), yankArg(args, O_PASS));
				if(acc == nullptr) {
					if(acc2 == nullptr) return ERR_NO_ACCOUNT;
					else acc = acc2;
				}
				buf = yankArg(args, O_CHANGE_M);
				if(buf == nullptr || !isalpha(*buf)) return ERR_NO_INFO;
				acc->middle = *buf;
				break;
			case O_CHANGE_SSN:
				acc = findAccount(people, yankArg(args, O_NUM), yankArg(args, O_PASS));
				if(acc == nullptr) {
					if(acc2 == nullptr) return ERR_NO_ACCOUNT;
					else acc = acc2;
				}
				buf = yankArg(args, O_CHANGE_SSN);
				if(buf == nullptr || !regex_match(buf, regex(R_SSN))) return ERR_NO_INFO;
				acc->social = atoi(buf);
				break;
			case O_TRANS: {
				acc = findAccount(people, yankArg(args, O_NUM), yankArg(args, O_PASS));
				if(acc == nullptr) return ERR_NO_ACCOUNT;
				acc2 = findAccount(people, yankArg(args, O_NUM), yankArg(args, O_PASS));
				if(acc2 == nullptr) return ERR_NO_TRANSFER_ACCOUNT;
				buf = yankArg(args, O_TRANS);
				if(buf == nullptr || !regex_match(buf, regex(R_NAME))) return ERR_NO_INFO;
				if(acc->balance < atoi(buf)) return ERR_TOO_MUCH_TRANSFER;
				acc->balance -= atoi(buf);
				acc2->balance += atoi(buf);
				break;
			}
			case O_NEWPASS:
				acc = findAccount(people, yankArg(args, O_NUM), yankArg(args, O_PASS));
				if(acc == nullptr) {
					if(acc2 == nullptr) return ERR_NO_ACCOUNT;
					else acc = acc2;
				}
				buf = yankArg(args, O_NEWPASS);
				if(buf == nullptr || !regex_match(buf, regex(R_PASS))) return ERR_NO_INFO;
				strcpy(acc->password, buf);
				break;
		}
		acc2 = acc;
		buf = nullptr;
	}

	for(pair<char, vector<char*>> arg : *args) {
		switch(arg.first) {
			case O_INFO:
				yankArg(args,O_INFO);
				acc2 = findAccount(people, yankArg(args, O_NUM), yankArg(args, O_PASS));
				if(acc2 == nullptr) acc2 = acc;
				if(acc2 == nullptr) return ERR_NO_ACCOUNT;
				displayInfo(acc2);
				break;
			case O_REPORT:
				if(!createReport(people, yankArg(args, O_REPORT))) return ERR_REPORT_FILE_ERR;
				break;	
		}
	}

	return 0;
}

/* -----------------------------------------------------------------------------
FUNCTION:          yankArg()
DESCRIPTION:       "Yanks" an argument value from the map, returning it and clearing it from the map
RETURNS:           The "yanked" value
NOTES:             Example:
                   [F] -> {blah, blah3}
                   [H] -> {blah2}
                   yankArg('F') -> returns blah

                   And then sets the map to this state:
                   [F] -> {blah3}
                   [H] -> {blah2}
                   yankArg('F') -> returns blah3
----------------------------------------------------------------------------- */
char* yankArg(map<char, vector<char*>>* args, char arg) {
	if(args->find(arg) == args->end()) return nullptr;
	if(args->at(arg).size() == 0) return nullptr;
	vector<char*>* vec = &args->at(arg);
	char* re = vec->at(0);
	vec->erase(vec->begin());
	return re;
}

/* -----------------------------------------------------------------------------
FUNCTION:          findAccount()
DESCRIPTION:       Finds an account based on the account number and password
RETURNS:           A pointer to the found account
----------------------------------------------------------------------------- */
Account* findAccount(vector<Account>* people, char* number, char* password) {
	if(number == nullptr || password == nullptr) return nullptr;
	for(Account& acc : *people) {
		if(!strcmp(acc.number, number) && !strcmp(acc.password, password)) return &acc;
	}
	return nullptr;
}

/* -----------------------------------------------------------------------------
FUNCTION:          helpMenu()
DESCRIPTION:       Displays a help menu which guides the user in how to use the program
RETURNS:           Void function
----------------------------------------------------------------------------- */
void helpMenu() {
	cout << "\tBank account management software version " << VERSION << endl
	     << "\tUsage:" << endl
		 << "\tbankacct [/" << O_HELP << "] - Display help menu" << endl
		 << "\tbankacct /D <action option> [info options] - Change or display information about an account" << "\n\n"
		 << "\tAction Options:" << endl
		 << "\t\t/" << O_CHANGE_AREA << " - Change the area code for a specified account" << endl
		 << "\t\t/" << O_CHANGE_F << " - Change the first name for a specified account" << endl
		 << "\t\t/" << O_CHANGE_PHONE << " - Change the phone number for a specified account" << endl
		 << "\t\t/" << O_CHANGE_L << " - Change the last name for a specified account" << endl
		 << "\t\t/" << O_CHANGE_M << " - Change the middle name for a specified account" << endl
		 << "\t\t/" << O_REPORT << " - Print a report to a specified report file" << endl
		 << "\t\t/" << O_CHANGE_SSN << " - Change the social security number for a specified account" << endl
		 << "\t\t/" << O_TRANS << " - Transfer money for one specified account to another" << endl 
		 << "\t\t/" << O_NEWPASS << " - Change the password for a specified account" << endl << endl
		 << "\tInfo options:" << endl
		 << "\t\t/" << O_NUM << " - specifies the account number for an action option" << endl
		 << "\t\t/" << O_PASS << " - specifies the password for an action option" << endl;
}

/* -----------------------------------------------------------------------------
FUNCTION:          displayInfo()
DESCRIPTION:       Displays the information of an account to the standard output
RETURNS:           Void function
----------------------------------------------------------------------------- */
void displayInfo(Account* acc) {
	cout << acc->first << endl
	     << acc->last << endl
		 << acc->middle << endl
		 << acc->social << endl
		 << acc->area << endl
		 << acc->phone << endl
		 << acc->balance << endl
		 << acc->number << endl
		 << acc->password << endl;
}

/* -----------------------------------------------------------------------------
FUNCTION:          createReport()
DESCRIPTION:       Creates a human-readable text file at a given file name
RETURNS:           Whether the report file was actually able to be created
----------------------------------------------------------------------------- */
bool createReport(vector<Account>* people, char* fileName) {
	ofstream file(fileName);
	if(!file.is_open()) {
		return false;
	}
	file << "-------  ----            -----           --  ---------  ------------  -------" << endl
	     << "Account  Last            First           MI  SS         Phone         Account" << endl
		 << "Number   Name            Name                Number     Number        Balance" << endl
		 << "-------  ----            -----           --  ---------  ------------  -------" << endl;
	
	for(Account& person : *people) {
		file <<  " " << person.number << "   "
		     << left << setw(14) << person.last << "  "
			 << setw(14) << person.first << "  "
			 << person.middle << ".  "
			 << person.social << "  "
		     << "(" << person.area << ")" << person.phone << "  "
			 << fixed << setprecision(2) << person.balance << endl;
	}
	return true;
}
/*----------------------------------------------------------------------------
FUNCTION:          loadDatabase()
DESCRIPTION:       Loads the database from a file
RETURNS:           Whether the database was able to be loaded
----------------------------------------------------------------------------- */
bool loadDatabase(vector<Account>* people, char* fileName) {
	ifstream input(fileName);
	if(!input.is_open()) return false;

	for(unsigned int i = 0; !input.eof(); i++) {
		Account person;
		input >> person.last
		      >> person.first
			  >> person.middle
			  >> person.social
			  >> person.area
			  >> person.phone
			  >> person.balance
			  >> person.number
			  >> person.password;
		person.nameLength = strlen(person.first) + strlen(person.last) + 4;
		if(input.eof()) break;
		people->push_back(person);
	}
	return true;
}
