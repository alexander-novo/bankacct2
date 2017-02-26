/* -----------------------------------------------------------------------------

FILE:              bankacct.h

DESCRIPTION:       Header file for my project. Contains constants, structures, and classes needed for main program

COMPILER:          g++ with c++ 11

MODIFICATION HISTORY:

Author                    Date               Version
---------------           ----------         --------------
Alexander Novotny         2016-10-18         1.0.0

----------------------------------------------------------------------------- */

#ifndef __BANKACCT_H__
#define __BANKACCT_H__

//Semvers
#define VERSION "1.0.0" 

#define FIRST_NAME_LENGTH 50
#define LAST_NAME_LENGTH 50
#define ACC_NUM_LENGTH 5
#define PASS_LENGTH 6

//Valid command line operator
#define SLASH '/'

//Command line options
//In order of priority
#define O_HELP '?'
#define O_DATA 'D'

#define O_CHANGE_AREA  'A'
#define O_CHANGE_F     'F'
#define O_CHANGE_PHONE 'H'
#define O_CHANGE_L     'L'
#define O_CHANGE_M     'M'
#define O_CHANGE_SSN   'S'
#define O_TRANS        'T'
#define O_NEWPASS      'W'

#define O_INFO         'I'
#define O_REPORT       'R'

#define O_NUM  'N'
#define O_PASS 'P'

//Field regex
#define R_AREA "^\\d{3}$"
#define R_NAME "^[:alpha:]*$"
#define R_MIDDLE "^[:alpha:]$"
#define R_PHONE "^\\d{7}$"
#define R_SSN "^\\d{9}$"
#define R_PASS "^[A-Z0-9]{6}$"


//Error codes
#define ERR_NO_DB 1
#define ERR_DB_NOT_FOUND 2
#define ERR_NO_ACCOUNT 3
#define ERR_NO_INFO 4
#define ERR_REPORT_FILE_ERR 5
#define ERR_NO_TRANSFER_ACCOUNT 6
#define ERR_TOO_MUCH_TRANSFER 7

using namespace std;

struct Account {
	char first[FIRST_NAME_LENGTH + 1];
	char last[LAST_NAME_LENGTH + 1];
	char middle;
	unsigned int social;
	unsigned int area;
	unsigned int phone;
	double balance;
	char number[ACC_NUM_LENGTH + 1];
	char password[PASS_LENGTH + 1];
	//Length of full name (including two spaces and a .)
	unsigned int nameLength;
};

class WriteOnShutdown {
	private:
		const char* filename;
		vector<Account>* database;
	public:
		WriteOnShutdown(char* a, vector<Account>* b) : filename(a), database(b) {}
		
		/* -----------------------------------------------------------------------------
		FUNCTION:          ~WriteOnShutdown()
		DESCRIPTION:       Destructor for WriteOnShutdown. When WriteOnShutdown gets deleted, 
                                   it writes the database to the specified output file
		RETURNS:           Void function
		----------------------------------------------------------------------------- */
		~WriteOnShutdown() {
			ofstream out(filename);
			for(Account& acc : *database) {
				out << acc.last<< endl
				    << acc.first << endl
					<< acc.middle << endl
					<< acc.social << endl
					<< acc.area << endl
					<< acc.phone << endl
					<< acc.balance << endl
					<< acc.number << endl
					<< acc.password << endl << endl;
			}
		}
};
#endif
