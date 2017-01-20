/*
Created 5 December 2014

Retail Merchandise Management Database

odbcsql.cpp by James R. Vaughan

Purposes:
Edit and view Microsoft SQL Server database in C++ using SQL queries
Provide interface for database of simulated retail store
Allows users different levels of access (admin, manager, and cashier).
Allows creation of new users with login credentials.
Allows users to create items, manage special sales pricing, manage inventory levels in the store and in the warehouse.
Allows users to create customer orders with multiple items and quantities of items.
Completion of the sale will affect current inventory levels.
Orders will also be stored in the database.

*/


#include "stdafx.h"
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <stdlib.h>
#include <sal.h>
#include <fstream>
#include <iostream>
#include <string>
#include <time.h>
#include <math.h>

using namespace std;
using std::string;


/*******************************************/
/* Macro to call ODBC functions and        */
/* report an error on failure.             */
/* Takes handle, handle type, and stmt     */
/*******************************************/

#define TRYODBC(h, ht, x)   {   RETCODE rc = x;\
                                if (rc != SQL_SUCCESS) \
								                                { \
                                    HandleDiagnosticRecord (h, ht, rc); \
								                                } \
                                if (rc == SQL_ERROR) \
								                                { \
                                    fwprintf(stderr, L"Error in " L#x L"\n"); \
                                    goto Exit;  \
								                                }  \
                            }
/******************************************/
/* Structure to store information about   */
/* a column.
/******************************************/

typedef struct STR_BINDING {
	SQLSMALLINT         cDisplaySize;           /* size to display  */
	WCHAR               *wszBuffer;             /* display buffer   */
	SQLLEN              indPtr;                 /* size or null     */
	BOOL                fChar;                  /* character col?   */
	struct STR_BINDING  *sNext;                 /* linked list      */
} BINDING;



/******************************************/
/* Forward references                     */
/******************************************/

void HandleDiagnosticRecord(SQLHANDLE      hHandle,
	SQLSMALLINT    hType,
	RETCODE        RetCode);

void DisplayResults(HSTMT       hStmt,
	SQLSMALLINT cCols);

void AllocateBindings(HSTMT         hStmt,
	SQLSMALLINT   cCols,
	BINDING**     ppBinding,
	SQLSMALLINT*  pDisplay);


void DisplayTitles(HSTMT    hStmt,
	DWORD    cDisplaySize,
	BINDING* pBinding);

void SetConsole(DWORD   cDisplaySize,
	BOOL    fInvert);

/*****************************************/
/* Some constants                        */
/*****************************************/


#define DISPLAY_MAX 50          // Arbitrary limit on column width to display
#define DISPLAY_FORMAT_EXTRA 3  // Per column extra display bytes (| <data> )
#define DISPLAY_FORMAT      L"%c %*.*s "
#define DISPLAY_FORMAT_C    L"%c %-*.*s "
#define NULL_SIZE           6   // <NULL>
#define SQL_QUERY_SIZE      1000 // Max. Num characters for SQL Query passed in.

#define PIPE                L'|'

SHORT   gHeight = 80;       // Users screen height



void DisplayMainMenu()
{

}


void ExecuteSQLQuery(SQLHENV     hEnv, SQLHDBC     hDbc, SQLHSTMT    hStmt, string SQLStatementToExecute)
{
	//SQLHENV     hEnv = NULL;
	//SQLHDBC     hDbc = NULL;
	//SQLHSTMT    hStmt = NULL;
	int SizeOfQuery = SQLStatementToExecute.size();

	WCHAR       wszInput[SQL_QUERY_SIZE]; // SQL_QUERY_SIZE


										  //while (_fgetts(wszInput, SQL_QUERY_SIZE - 1, stdin))



	for (int index = 0; index < SizeOfQuery; index++)
	{
		wszInput[index] = SQLStatementToExecute[index];
	}
	for (int index = SizeOfQuery; index < SQL_QUERY_SIZE; index++)
	{
		wszInput[index] = NULL;
	}


	//_fgetts(wszInput, SQL_QUERY_SIZE - 1, stdin);




	{
		RETCODE     RetCode;
		SQLSMALLINT sNumResults;

		// Execute the query

		if (!(*wszInput))
		{
			//wprintf(L"SQL COMMAND>");
			//continue;
		}
		RetCode = SQLExecDirect(hStmt, wszInput, SQL_NTS);

		switch (RetCode)
		{
		case SQL_SUCCESS_WITH_INFO:
		{
			HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
			// fall through
		}
		case SQL_SUCCESS:
		{
			// If this is a row-returning query, display
			// results
			TRYODBC(hStmt,
				SQL_HANDLE_STMT,
				SQLNumResultCols(hStmt, &sNumResults));

			if (sNumResults > 0)
			{
				DisplayResults(hStmt, sNumResults);
			}
			else
			{
				SQLLEN cRowCount;

				TRYODBC(hStmt,
					SQL_HANDLE_STMT,
					SQLRowCount(hStmt, &cRowCount));

				if (cRowCount >= 0)
				{
					wprintf(L"%Id %s affected\n",
						cRowCount,
						cRowCount == 1 ? L"row" : L"rows");
				}
			}
			break;
		}

		case SQL_ERROR:
		{
			HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
			break;
		}

		default:
			fwprintf(stderr, L"Unexpected return code %hd!\n", RetCode);

		}
		TRYODBC(hStmt,
			SQL_HANDLE_STMT,
			SQLFreeStmt(hStmt, SQL_CLOSE));

		//wprintf(L"SQL COMMAND>");
	}
Exit:
	if (hStmt)
	{
	}
	if (hDbc)
	{
	}
	if (hEnv)
	{
	}


	//Exit:
	//
	//	// Free ODBC handles and exit
	//
	//	if (hStmt)
	//	{
	//		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	//	}
	//
	//	if (hDbc)
	//	{
	//		SQLDisconnect(hDbc);
	//		SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	//	}
	//
	//	if (hEnv)
	//	{
	//		SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
	//	}
	//
	//	wprintf(L"\nDisconnected.");
}




string AddEmployee()
{
	string EmployeeID;
	string CashierNo;
	string FirstName;
	string LastName;

	string ReturnSQLStatement = "INSERT INTO Employee VALUES ('";

	cout << "  New employee information:" << endl;
	cout << "  New Employee ID (7 numerical digits): >> ";
	std::getline(std::cin, EmployeeID);
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  New Employee Cashier Number (4 numerical digits): >> ";
	std::getline(std::cin, CashierNo);
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  New Employee First Name: >> ";
	std::getline(std::cin, FirstName);
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  New Employee Last Name: >> ";
	std::getline(std::cin, LastName);
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	ReturnSQLStatement.append(EmployeeID);
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(CashierNo);
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(FirstName);
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(LastName);
	ReturnSQLStatement.append("')");

	return ReturnSQLStatement;
}


string EditEmployee(string EmployeeID)
{
	string CashierNo;
	string FirstName;
	string LastName;

	string ReturnSQLStatement = "UPDATE Employee SET CashierNo = '";

	cout << "  Employee Cashier Number (4 numerical digits): >> ";
	std::getline(std::cin, CashierNo);
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Employee First Name: >> ";
	std::getline(std::cin, FirstName);
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Employee Last Name: >> ";
	std::getline(std::cin, LastName);
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	ReturnSQLStatement.append(CashierNo);
	ReturnSQLStatement.append("', FirstName = '");
	ReturnSQLStatement.append(FirstName);
	ReturnSQLStatement.append("', LastName = '");
	ReturnSQLStatement.append(LastName);
	ReturnSQLStatement.append("' WHERE EmployeeID = ");
	ReturnSQLStatement.append(EmployeeID);
	ReturnSQLStatement.append(";");


	return ReturnSQLStatement;
}



string AddLowSaleItem()
{
	string SKU;
	int MarkdownAmount;

	string ReturnSQLStatement = "INSERT INTO LowSaleItems VALUES ('";

	cout << "  New low sale item information:" << endl;
	cout << "  SKU of low sale item to mark down: >> ";
	cin >> SKU;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Enter markdown amount(0-100, percent taken off): >> ";
	cin >> MarkdownAmount;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	ReturnSQLStatement.append(SKU);
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(std::to_string(MarkdownAmount));
	ReturnSQLStatement.append("')");

	return ReturnSQLStatement;
}



string EditLowSaleItem(string SKU)
{
	int MarkdownAmount;

	string ReturnSQLStatement = "UPDATE LowSaleItems SET MarkdownAmount = '";

	cout << "  Enter markdown amount(0-100, percent taken off): >> ";
	cin >> MarkdownAmount;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	ReturnSQLStatement.append(std::to_string(MarkdownAmount));
	ReturnSQLStatement.append("' WHERE SKU = '");
	ReturnSQLStatement.append(SKU);
	ReturnSQLStatement.append("';");


	return ReturnSQLStatement;
}



string AddWarehouseItem()
{
	string SKU;
	int OnHands;

	string ReturnSQLStatement = "INSERT INTO Warehouse2 VALUES ('";

	cout << "  New warehouse item information:" << endl;
	cout << "  SKU of new warehouse item: >> ";
	cin >> SKU;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Enter current onhands in the warehouse (zero is fine): >> ";
	cin >> OnHands;
	cin.clear();
	cin.ignore(INT_MAX, '\n');


	ReturnSQLStatement.append(SKU);
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(std::to_string(OnHands));
	ReturnSQLStatement.append("')");

	return ReturnSQLStatement;
}


string EditWarehouseItem(string SKU)
{
	int OnHands;

	string ReturnSQLStatement = "UPDATE Warehouse2 SET OnHands = '";

	cout << "  Current onhands (zero is fine): >> ";
	cin >> OnHands;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	ReturnSQLStatement.append(std::to_string(OnHands));
	ReturnSQLStatement.append("' WHERE SKU = '");
	ReturnSQLStatement.append(SKU);
	ReturnSQLStatement.append("';");


	return ReturnSQLStatement;
}


string AddInventoryItem()
{
	string SKU;
	int OnHands;
	int ShelfLimit;
	int OrderMoreAmount;

	string ReturnSQLStatement = "INSERT INTO Inventory VALUES ('";

	cout << "  New inventory item information:" << endl;
	cout << "  SKU of new inventory item: >> ";
	cin >> SKU;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Enter current onhands (zero is fine): >> ";
	cin >> OnHands;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Shelf limit (zero is fine): >> ";
	cin >> ShelfLimit;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Order more amount (zero is fine): >> ";
	cin >> OrderMoreAmount;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	ReturnSQLStatement.append(SKU);
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(std::to_string(OnHands));
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(std::to_string(ShelfLimit));
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(std::to_string(OrderMoreAmount));
	ReturnSQLStatement.append("')");

	return ReturnSQLStatement;
}



string EditInventoryItem(string SKU)
{
	int OnHands;
	int ShelfLimit;
	int OrderMoreAmount;



	string ReturnSQLStatement = "UPDATE Inventory SET OnHands = '";

	cout << "  Current onhands (zero is fine): >> ";
	cin >> OnHands;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Shelf limit (zero is fine): >> ";
	cin >> ShelfLimit;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Order more amount (zero is fine): >> ";
	cin >> OrderMoreAmount;
	cin.clear();
	cin.ignore(INT_MAX, '\n');


	ReturnSQLStatement.append(std::to_string(OnHands));
	ReturnSQLStatement.append("', ShelfLimit = '");
	ReturnSQLStatement.append(std::to_string(ShelfLimit));
	ReturnSQLStatement.append("', OrderMoreAmount = '");
	ReturnSQLStatement.append(std::to_string(OrderMoreAmount));
	ReturnSQLStatement.append("' WHERE SKU = '");
	ReturnSQLStatement.append(SKU);
	ReturnSQLStatement.append("';");


	return ReturnSQLStatement;
}



string CreateItem()
{
	string SKU;
	string Description;
	double Price;
	double Weight;
	int CasePack;
	string Department;

	string ReturnSQLStatement = "INSERT INTO Item VALUES ('";

	cout << "  New item information:" << endl;
	cout << "  SKU: >> ";
	cin >> SKU;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Description: >> ";
	std::getline(std::cin, Description);
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Price: >> $";
	cin >> Price;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Weight: >> ";
	cin >> Weight;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Case Pack: >> ";
	cin >> CasePack;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Department: >> ";
	std::getline(std::cin, Department);
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	ReturnSQLStatement.append(SKU);
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(Description);
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(std::to_string(Price));
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(std::to_string(Weight));
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(std::to_string(CasePack));
	ReturnSQLStatement.append("','");
	ReturnSQLStatement.append(Department);
	ReturnSQLStatement.append("')");

	return ReturnSQLStatement;
}



string EditItem(string SKU)
{
	string Description;
	double Price;
	double Weight;
	int CasePack;
	string Department;



	string ReturnSQLStatement = "UPDATE Item SET Description = '";



	cout << "  Description: >> ";
	std::getline(std::cin, Description);
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Price: >> $";
	cin >> Price;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Weight: >> ";
	cin >> Weight;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Case Pack: >> ";
	cin >> CasePack;
	cin.clear();
	cin.ignore(INT_MAX, '\n');

	cout << "  Department: >> ";
	std::getline(std::cin, Department);
	cin.clear();
	cin.ignore(INT_MAX, '\n');


	ReturnSQLStatement.append(Description);
	ReturnSQLStatement.append("', Price = '");
	ReturnSQLStatement.append(std::to_string(Price));
	ReturnSQLStatement.append("', Weight = '");
	ReturnSQLStatement.append(std::to_string(Weight));
	ReturnSQLStatement.append("', CasePack = '");
	ReturnSQLStatement.append(std::to_string(CasePack));
	ReturnSQLStatement.append("', Department = '");
	ReturnSQLStatement.append(Department);
	ReturnSQLStatement.append("' WHERE SKU = '");
	ReturnSQLStatement.append(SKU);
	ReturnSQLStatement.append("';");


	return ReturnSQLStatement;
}



int __cdecl wmain(int argc, _In_reads_(argc) WCHAR **argv)
{
	SQLHENV     hEnv = NULL;
	SQLHDBC     hDbc = NULL;
	SQLHSTMT    hStmt = NULL;
	WCHAR*      pwszConnStr;
	//WCHAR       wszInput[SQL_QUERY_SIZE];


	string MainMenuInput;
	string Input1;
	string Input2;
	string Input3;
	string Input4;
	string Input5;


	string DBUsername;
	string DBPassword;
	bool isLoggedIn = false;
	bool WasValidResponse = true;

	bool AdminPrivileges = false;
	bool ManagerPrivileges = false;
	bool CashierPrivileges = false;

	string DisplayStatus = "mainmenu";

	const int ADMINLOGINSLENGTH = 1;
	string AdminLogins[ADMINLOGINSLENGTH];
	AdminLogins[0] = "admin";

	const int ADMINPASSWORDSLENGTH = 1;
	string AdminPasswords[ADMINPASSWORDSLENGTH];
	AdminPasswords[0] = "adminpassword";

	const int MANAGERLOGINSLENGTH = 1;
	string ManagerLogins[MANAGERLOGINSLENGTH];
	ManagerLogins[0] = "manager";

	const int MANAGERPASSWORDSLENGTH = 1;
	string ManagerPasswords[MANAGERPASSWORDSLENGTH];
	ManagerPasswords[0] = "managerpassword";

	const int CASHIERLOGINSLENGTH = 1;
	string CashierLogins[CASHIERLOGINSLENGTH];
	CashierLogins[0] = "cashier";

	const int CASHIERPASSWORDSLENGTH = 1;
	string CashierPasswords[CASHIERPASSWORDSLENGTH];
	CashierPasswords[0] = "cashierpassword";



	// Allocate an environment

	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_ERROR)
	{
		fwprintf(stderr, L"Unable to allocate an environment handle\n");
		exit(-1);
	}

	// Register this as an application that expects 3.x behavior,
	// you must register something if you use AllocHandle

	TRYODBC(hEnv,
		SQL_HANDLE_ENV,
		SQLSetEnvAttr(hEnv,
			SQL_ATTR_ODBC_VERSION,
			(SQLPOINTER)SQL_OV_ODBC3,
			0));

	// Allocate a connection
	TRYODBC(hEnv,
		SQL_HANDLE_ENV,
		SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc));

	if (argc > 1)
	{
		pwszConnStr = *++argv;
	}
	else
	{
		pwszConnStr = L"";
	}

	// Connect to the driver.  Use the connection string if supplied
	// on the input, otherwise let the driver manager prompt for input.

	TRYODBC(hDbc,
		SQL_HANDLE_DBC,
		SQLDriverConnect(hDbc,
			GetDesktopWindow(),
			pwszConnStr,
			SQL_NTS,
			NULL,
			0,
			NULL,
			SQL_DRIVER_COMPLETE));

	fwprintf(stderr, L"Connected!\n");

	TRYODBC(hDbc,
		SQL_HANDLE_DBC,
		SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt));

	//wprintf(L"Enter SQL commands, type (control)Z to exit\nSQL COMMAND>");

	//
	// Login Screen
	//
	while (isLoggedIn == false)
	{
		cout << endl;
		cout << "  RETAIL MERCHANDISE MANAGEMENT DATABASE" << endl << endl << endl;
		cout << "  James Vaughan and Keith Robinson" << endl << endl << endl;
		cout << "  Username: >> ";
		cin >> DBUsername;
		cin.clear();
		cin.ignore(INT_MAX, '\n');
		//cout << endl;

		cout << "  Password: >> ";
		cin >> DBPassword;
		cin.clear();
		cin.ignore(INT_MAX, '\n');
		cout << endl << endl;

		//
		// Check for admin login
		// 

		for (int kk = 0; kk < ADMINLOGINSLENGTH; kk++)
		{
			if (AdminLogins[kk] == DBUsername)
			{
				if (AdminPasswords[kk] == DBPassword)
				{
					AdminPrivileges = true;
					ManagerPrivileges = true;
					CashierPrivileges = true;
					isLoggedIn = true;
				}
			}

		}


		//
		// Check for manager login
		// 

		for (int kk = 0; kk < MANAGERLOGINSLENGTH; kk++)
		{
			if (ManagerLogins[kk] == DBUsername)
			{
				if (ManagerPasswords[kk] == DBPassword)
				{
					AdminPrivileges = false;
					ManagerPrivileges = true;
					CashierPrivileges = true;
					isLoggedIn = true;
				}
			}

		}


		//
		// Check for cashier login
		// 

		for (int kk = 0; kk < CASHIERLOGINSLENGTH; kk++)
		{
			if (CashierLogins[kk] == DBUsername)
			{
				if (CashierPasswords[kk] == DBPassword)
				{
					AdminPrivileges = false;
					ManagerPrivileges = false;
					CashierPrivileges = true;
					isLoggedIn = true;
				}
			}

		}

		system("cls");

		if (isLoggedIn == false)
		{
			cout << "ERROR: INCORRECT USERNAME OR PASSWORD" << endl;
		}
	}

	//
	// Primary Program
	//
	while (isLoggedIn == true)
	{
		//
		// Main Menu
		//
		while (DisplayStatus == "mainmenu")
		{
			cout << endl;
			cout << "  MAIN MENU:" << endl << endl << endl;

			if (AdminPrivileges == true)
			{
				cout << "   1. Inventory Management" << endl;
				cout << "   2. Item Management" << endl;
				cout << "   3. Warehouse Management" << endl;
				cout << endl;
			}


			if (ManagerPrivileges == true)
			{
				cout << "   4. Inventory Report" << endl;
				cout << "   5. Onhand Adjustment" << endl;
				cout << "   6. Low Sale Item Management" << endl;
				cout << "   7. Employee Managment" << endl;
				cout << "   8. View Transaction Log" << endl;
				cout << endl;
			}

			if (CashierPrivileges == true)
			{
				cout << "   9. Create Transaction" << endl;
				cout << endl;
			}

			cout << endl;
			cout << "  >> ";

			/*
			std::getline(std::cin, MainMenuInput);
			cin.clear();
			cin.ignore(INT_MAX, '\n');
			*/

			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			//
			// Admin options:
			//
			if (MainMenuInput == "1" && AdminPrivileges == true)
			{
				DisplayStatus = "inventorymanagement";
			}
			else if (MainMenuInput == "2" && AdminPrivileges == true)
			{
				DisplayStatus = "itemmanagement";
			}

			else if (MainMenuInput == "3" && AdminPrivileges == true)
			{
				DisplayStatus = "warehousemanagement";
			}

			//
			// Manager Options:
			//
			else if (MainMenuInput == "4" && ManagerPrivileges == true)
			{
				DisplayStatus = "inventoryreport";
			}
			else if (MainMenuInput == "5" && ManagerPrivileges == true)
			{
				DisplayStatus = "onhandadjustment";
			}
			else if (MainMenuInput == "6" && ManagerPrivileges == true)
			{
				DisplayStatus = "lowsaleitemmanagement";
			}
			else if (MainMenuInput == "7" && ManagerPrivileges == true)
			{
				DisplayStatus = "employeemanagement";
			}
			else if (MainMenuInput == "8" && ManagerPrivileges == true)
			{
				DisplayStatus = "viewtransactionlog";
			}


			//
			// Cashier Options:
			//
			else if (MainMenuInput == "9" && CashierPrivileges == true)
			{
				DisplayStatus = "createtransaction";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}

		}



		//
		// Inventory Management
		//
		while (DisplayStatus == "inventorymanagement")
		{
			cout << endl;
			cout << "  INVENTORY MANAGEMENT:" << endl << endl << endl;
			cout << "   1. View Inventory" << endl;
			cout << "   2. Add Inventory Item" << endl;
			cout << "   3. Edit Inventory Item" << endl;
			cout << "   4. Remove Inventory Item" << endl;
			cout << endl;
			cout << "   5. Main Menu" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "viewinventory";
			}
			else if (MainMenuInput == "2")
			{
				DisplayStatus = "addinventoryitem";
			}
			else if (MainMenuInput == "3")
			{
				DisplayStatus = "editinventoryitem";
			}
			else if (MainMenuInput == "4")
			{
				DisplayStatus = "removeinventoryitem";
			}
			else if (MainMenuInput == "5")
			{
				DisplayStatus = "mainmenu";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Inventory Management: View Inventory
		//
		while (DisplayStatus == "viewinventory")
		{
			cout << endl;
			cout << "  INVENTORY MANAGEMENT: VIEW INVENTORY" << endl << endl << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");


			cout << endl;
			cout << "   1. Inventory Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "inventorymanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Inventory Management: Add Inventory Item
		//
		while (DisplayStatus == "addinventoryitem")
		{
			cout << endl;
			cout << "  INVENTORY MANAGEMENT: ADD INVENTORY ITEM" << endl << endl << endl;

			cout << "Items:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Item");
			cout << endl;
			cout << "Items currently in inventory:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");
			cout << endl;

			Input1 = AddInventoryItem();
			cout << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);


			cout << endl;
			cout << "   1. Add Another Inventory Item" << endl;
			cout << endl;
			cout << "   2. Inventory Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "addinventoryitem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "inventorymanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Inventory Management: Edit Inventory Item
		//
		while (DisplayStatus == "editinventoryitem")
		{
			cout << endl;
			cout << "  INVENTORY MANAGEMENT: EDIT INVENTORY ITEM" << endl << endl << endl;

			cout << "  Edit inventory item information:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");
			cout << endl;
			cout << "  Enter SKU of inventory item to edit: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "SELECT * FROM Inventory WHERE SKU = ";
			Input3.append(Input2);

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);

			Input1 = EditInventoryItem(Input2);
			cout << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);


			cout << endl;
			cout << "   1. Edit Another Inventory Item" << endl;
			cout << endl;
			cout << "   2. Inventory Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "editinventoryitem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "inventorymanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Inventory Management: Remove Inventory Item
		//
		while (DisplayStatus == "removeinventoryitem")
		{
			cout << endl;
			cout << "  INVENTORY MANAGEMENT: REMOVE INVENTORY ITEM" << endl << endl << endl;

			cout << "  Enter inventory item to delete:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");
			cout << endl;
			cout << "  Enter SKU of item to remove: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "DELETE FROM Inventory WHERE SKU = ";
			Input3.append(Input2);

			cout << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);


			cout << endl;
			cout << "   1. Remove Another Inventory Item" << endl;
			cout << endl;
			cout << "   2. Inventory Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "removeinventoryitem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "inventorymanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}


		//
		// Item Management
		//
		while (DisplayStatus == "itemmanagement")
		{
			cout << endl;
			cout << "  ITEM MANAGEMENT:" << endl << endl << endl;
			cout << "   1. View Items" << endl;
			cout << "   2. Create Item" << endl;
			cout << "   3. Edit Item" << endl;
			cout << "   4. Delete Item" << endl;
			cout << endl;
			cout << "   5. Main Menu" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "viewitems";
			}
			else if (MainMenuInput == "2")
			{
				DisplayStatus = "createitem";
			}
			else if (MainMenuInput == "3")
			{
				DisplayStatus = "edititem";
			}
			else if (MainMenuInput == "4")
			{
				DisplayStatus = "deleteitem";
			}


			else if (MainMenuInput == "5")
			{
				DisplayStatus = "mainmenu";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Item Management: View Items
		//
		while (DisplayStatus == "viewitems")
		{
			cout << endl;
			cout << "  ITEM MANAGEMENT: VIEW ITEMS" << endl << endl << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Item");


			cout << endl;
			cout << "   1. Item Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "itemmanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Item Management: Create Item
		//
		while (DisplayStatus == "createitem")
		{
			cout << endl;
			cout << "  ITEM MANAGEMENT: CREATE ITEM" << endl << endl << endl;

			Input1 = CreateItem();

			cout << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);


			cout << endl;
			cout << "   1. Create Another Item" << endl;
			cout << endl;
			cout << "   2. Item Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "createitem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "itemmanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Item Management: Edit Item
		//
		while (DisplayStatus == "edititem")
		{
			cout << endl;
			cout << "  ITEM MANAGEMENT: EDIT ITEM" << endl << endl << endl;

			cout << "  Edit item information:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Item");
			cout << endl;
			cout << "  Enter SKU of item to edit: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "SELECT * FROM Item WHERE SKU = ";
			Input3.append(Input2);

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);

			Input1 = EditItem(Input2);
			cout << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);


			cout << endl;
			cout << "   1. Edit Another Item" << endl;
			cout << endl;
			cout << "   2. Item Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "edititem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "itemmanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Item Management: Delete Item
		//
		while (DisplayStatus == "deleteitem")
		{
			cout << endl;
			cout << "  ITEM MANAGEMENT: DELETE ITEM" << endl << endl << endl;

			cout << "  Enter item to delete:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Item");
			cout << endl;
			cout << "  Enter SKU of item to delete: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "DELETE FROM Item WHERE SKU = ";
			Input3.append(Input2);

			cout << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);


			cout << endl << endl;
			cout << "   1. Delete Another Item" << endl;
			cout << endl;
			cout << "   2. Item Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "deleteitem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "itemmanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}


		//
		// Warehouse Management
		//
		while (DisplayStatus == "warehousemanagement")
		{
			cout << endl;
			cout << "  WAREHOUSE MANAGEMENT:" << endl << endl << endl;
			cout << "   1. View Warehouse Items" << endl;
			cout << "   2. Add Warehouse Item" << endl;
			cout << "   3. Edit Warehouse Item" << endl;
			cout << "   4. Delete Warehouse Item" << endl;
			cout << endl;
			cout << "   5. Main Menu" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "viewwarehouseitems";
			}
			else if (MainMenuInput == "2")
			{
				DisplayStatus = "addwarehouseitem";
			}
			else if (MainMenuInput == "3")
			{
				DisplayStatus = "editwarehouseitem";
			}
			else if (MainMenuInput == "4")
			{
				DisplayStatus = "deletewarehouseitem";
			}
			else if (MainMenuInput == "5")
			{
				DisplayStatus = "mainmenu";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Warehouse Management: View Warehouse Items
		//
		while (DisplayStatus == "viewwarehouseitems")
		{
			cout << endl;
			cout << "  WAREHOUSE MANAGEMENT: VIEW WAREHOUSE ITEMS" << endl << endl << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Warehouse2 LEFT OUTER JOIN Item ON Warehouse2.SKU = Item.SKU");


			cout << endl;
			cout << "   1. Warehouse Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "warehousemanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Warehouse Managment: Add Warehouse Item
		//
		while (DisplayStatus == "addwarehouseitem")
		{
			cout << endl;
			cout << "  WAREHOUSE MANAGEMENT: ADD WAREHOUSE ITEM" << endl << endl << endl;

			cout << "Items:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Item");
			cout << endl;
			cout << "Items currently in inventory:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");
			cout << endl;
			cout << "Items currently in warehouse:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Warehouse2 LEFT OUTER JOIN Item ON Warehouse2.SKU = Item.SKU");
			cout << endl;


			Input1 = AddWarehouseItem();
			cout << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);


			cout << endl << endl;
			cout << "   1. Add Another Item" << endl;
			cout << endl;
			cout << "   2. Warehouse Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "addwarehouseitem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "warehousemanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Warehouse Management: Edit Warehouse Item
		//
		while (DisplayStatus == "editwarehouseitem")
		{
			cout << endl;
			cout << "  WAREHOUSE MANAGEMENT: EDIT WAREHOUSE ITEM" << endl << endl << endl;

			cout << "Items currently in inventory:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");
			cout << endl;
			cout << "Items currently in warehouse:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Warehouse2 LEFT OUTER JOIN Item ON Warehouse2.SKU = Item.SKU");
			cout << endl;
			cout << "  Enter SKU of warehouse item to edit: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "SELECT * FROM Warehouse2 LEFT OUTER JOIN Item ON Warehouse2.SKU = Item.SKU WHERE Warehouse2.SKU = ";
			Input3.append(Input2);

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);

			Input1 = EditWarehouseItem(Input2);

			cout << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);
			cout << endl;

			cout << "   1. Edit Another Warehouse Item" << endl;
			cout << endl;
			cout << "   2. Warehouse Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "editwarehouseitem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "warehousemanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Warehouse Management: Delete Warehouse Item
		//
		while (DisplayStatus == "deletewarehouseitem")
		{
			cout << endl;
			cout << "  WAREHOUSE MANAGEMENT: DELETE WAREHOUSE ITEM" << endl << endl << endl;

			cout << "Items currently in inventory:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");
			cout << endl;
			cout << "Items currently in warehouse:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Warehouse2 LEFT OUTER JOIN Item ON Warehouse2.SKU = Item.SKU");
			cout << endl;

			cout << "  Enter SKU of item to delete: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "DELETE FROM Warehouse2 WHERE SKU = ";
			Input3.append(Input2);

			cout << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);


			cout << endl << endl;
			cout << "   1. Delete Another Warehouse Item" << endl;
			cout << endl;
			cout << "   2. Warehouse Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "deletewarehouseitem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "warehousemanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}




		//
		// Inventory Report
		//
		while (DisplayStatus == "inventoryreport")
		{
			cout << endl;
			cout << "  INVENTORY REPORT:" << endl << endl << endl;

			cout << "Items not in use (not in warehouse and not in inventory): " << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Item WHERE Item.SKU NOT IN (SELECT SKU FROM Inventory) AND Item.SKU NOT IN (SELECT SKU FROM Warehouse2)");
			//SELECT * FROM LowSaleItems LEFT OUTER JOIN Item ON LowSaleItems.SKU = Item.SKU

			cout << endl;
			cout << "Items in warehouse: " << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Warehouse2 LEFT OUTER JOIN Item ON Warehouse2.SKU = Item.SKU");
			//SELECT * FROM LowSaleItems LEFT OUTER JOIN Item ON LowSaleItems.SKU = Item.SKU

			cout << endl;
			cout << "Items in inventory, with corresponding markdowns: " << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU LEFT OUTER JOIN LowSaleItems ON Inventory.SKU = LowSaleItems.SKU");
			//SELECT * FROM LowSaleItems LEFT OUTER JOIN Item ON LowSaleItems.SKU = Item.SKU


			cout << endl;
			cout << "   1. Main Menu" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "mainmenu";
			}
			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}



		//
		// Onhand Adjustment
		//
		while (DisplayStatus == "onhandadjustment")
		{
			cout << endl;
			cout << "  ONHAND ADJUSTMENT:" << endl << endl << endl;
			cout << "   1. Order Item from Warehouse" << endl;
			cout << "   2. Return Item to Warehouse" << endl;
			cout << "   3. Return Damaged Item / Claim Stolen Item" << endl;
			cout << endl;
			cout << "   4. Main Menu" << endl;
			cout << endl;
			cout << "  >> ";

			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "orderitems";
			}
			else if (MainMenuInput == "2")
			{
				DisplayStatus = "returnitems";
			}
			else if (MainMenuInput == "3")
			{
				DisplayStatus = "damagedorstolenitems";
			}
			else if (MainMenuInput == "4")
			{
				DisplayStatus = "mainmenu";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Onhand Adjustment: Order Item From Warehouse
		//
		while (DisplayStatus == "orderitems")
		{
			cout << endl;
			cout << "  ONHAND ADJUSTMENT: ORDER ITEM FROM WAREHOUSE" << endl << endl << endl;

			cout << "Items currently in warehouse:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Warehouse2 LEFT OUTER JOIN Item ON Warehouse2.SKU = Item.SKU");
			cout << endl;
			cout << "Items currently in inventory:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");
			cout << endl;
			cout << "Items which are below the order more amount, and their corresponding onhands in warehouse:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Warehouse2 ON Inventory.SKU = Warehouse2.SKU WHERE (Inventory.OnHands < Inventory.OrderMoreAmount)");
			cout << endl;

			//Input1 = AddLowSaleItem();
			//cout << endl;
			//ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);

			cout << endl;
			cout << "  Enter SKU of item you wish to order: >> ";
			cin >> Input1;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl;
			cout << "  Enter Quantity of the item that you wish to order: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "BEGIN TRY UPDATE Warehouse2 SET OnHands = (Onhands - ";
			Input3.append(Input2);
			Input3.append(") WHERE SKU = ");
			Input3.append(Input1);

			Input3.append("; UPDATE Inventory SET OnHands = (Onhands + ");
			Input3.append(Input2);
			Input3.append(") WHERE SKU = ");
			Input3.append(Input1);
			Input3.append("; END TRY BEGIN CATCH END CATCH");

			cout << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);


			cout << endl << endl;
			cout << "   1. Order Another Item" << endl;
			cout << endl;
			cout << "   2. Onhands Adustment" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "orderitems";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "onhandadjustment";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Onhand Adjustment: Return Item to Warehouse
		//
		while (DisplayStatus == "returnitems")
		{
			cout << endl;
			cout << "  ONHAND ADJUSTMENT: RETURN ITEM TO WAREHOUSE" << endl << endl << endl;

			cout << "Items currently in warehouse:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Warehouse2 LEFT OUTER JOIN Item ON Warehouse2.SKU = Item.SKU");
			cout << endl;
			cout << "Items currently in inventory:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");
			cout << endl;
			cout << "Items which are above the shelf limit amount, and their corresponding onhands in warehouse:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Warehouse2 ON Inventory.SKU = Warehouse2.SKU WHERE (Inventory.OnHands > Inventory.ShelfLimit)");
			cout << endl;

			//Input1 = AddLowSaleItem();
			//cout << endl;
			//ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);

			cout << endl;
			cout << "  Enter SKU of item you wish to return: >> ";
			cin >> Input1;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl;
			cout << "  Enter Quantity of the item that you wish to return: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "SET XACT_ABORT ON BEGIN TRANSACTION T1 UPDATE Warehouse2 SET OnHands = (Onhands + ";
			Input3.append(Input2);
			Input3.append(") WHERE SKU = ");
			Input3.append(Input1);

			Input3.append(" UPDATE Inventory SET OnHands = (Onhands - ");
			Input3.append(Input2);
			Input3.append(") WHERE SKU = ");
			Input3.append(Input1);
			Input3.append(" COMMIT TRANSACTION T1;");

			cout << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);


			cout << endl << endl;
			cout << "   1. Return Another Item" << endl;
			cout << endl;
			cout << "   2. Onhands Adustment" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "returnitems";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "onhandadjustment";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Onhand Adjustment: Return Damaged Item / Claim Stolen Item
		//
		while (DisplayStatus == "damagedorstolenitems")
		{
			cout << endl;
			cout << "  ONHAND ADJUSTMENT: RETURN DAMAGED ITEM / CLAIM STOLEN ITEM" << endl << endl << endl;

			cout << "Items currently in inventory:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");
			cout << endl;

			//Input1 = AddLowSaleItem();
			//cout << endl;
			//ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);

			cout << endl;
			cout << "  Enter SKU of damaged/stolen item you wish to return or claim: >> ";
			cin >> Input1;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl;
			cout << "  Enter Quantity of the item that you wish to return or claim: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "SET XACT_ABORT ON BEGIN TRANSACTION T1 UPDATE Inventory SET OnHands = (Onhands - ";
			Input3.append(Input2);
			Input3.append(") WHERE SKU = ");
			Input3.append(Input1);
			Input3.append(" COMMIT TRANSACTION T1;");
			cout << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);


			cout << endl << endl;
			cout << "   1. Return or Claim Another Item" << endl;
			cout << endl;
			cout << "   2. Onhands Adustment" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "damagedorstolenitems";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "onhandadjustment";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}



		//
		// Low Sale Item Management
		//
		while (DisplayStatus == "lowsaleitemmanagement")
		{
			cout << endl;
			cout << "  LOW SALE ITEM MANAGEMENT:" << endl << endl << endl;
			cout << "   1. View Low Sale Item Markdowns" << endl;
			cout << "   2. Add Low Sale Item Markdowns" << endl;
			cout << "   3. Edit Low Sale Item Markdowns" << endl;
			cout << "   4. Remove Low Sale Item Markdowns" << endl;
			cout << endl;
			cout << "   5. Main Menu" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "viewlowsaleitems";
			}
			else if (MainMenuInput == "2")
			{
				DisplayStatus = "addlowsaleitem";
			}
			else if (MainMenuInput == "3")
			{
				DisplayStatus = "editlowsaleitem";
			}
			else if (MainMenuInput == "4")
			{
				DisplayStatus = "removelowsaleitem";
			}
			else if (MainMenuInput == "5")
			{
				DisplayStatus = "mainmenu";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Low Sale Item Management: View Low Sale Item Markdowns
		//
		while (DisplayStatus == "viewlowsaleitems")
		{
			cout << endl;
			cout << "  LOW SALE ITEM MANAGEMENT: VIEW LOW SALE ITEM MARKDOWNS" << endl << endl << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM LowSaleItems LEFT OUTER JOIN Item ON LowSaleItems.SKU = Item.SKU");


			cout << endl;
			cout << "   1. Low Sale Item Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "lowsaleitemmanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Low Sale Item Managment: Add Low Sale Item
		//
		while (DisplayStatus == "addlowsaleitem")
		{
			cout << endl;
			cout << "  LOW SALE ITEM MANAGEMENT: ADD LOW SALE ITEM MARKDOWN" << endl << endl << endl;

			cout << "Items currently in inventory:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");
			cout << endl;
			cout << "Items currently in with a markdown:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM LowSaleItems LEFT OUTER JOIN Item ON LowSaleItems.SKU = Item.SKU");
			cout << endl;


			Input1 = AddLowSaleItem();
			cout << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);


			cout << endl << endl;
			cout << "   1. Add Another Low Sale Item Markdown" << endl;
			cout << endl;
			cout << "   2. Low Sale Item Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "addlowsaleitem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "lowsaleitemmanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Low Sale Item Management: Edit Low Sale Item
		//
		while (DisplayStatus == "editlowsaleitem")
		{
			cout << endl;
			cout << "  LOW SALE ITEM MANAGEMENT: EDIT LOW SALE ITEM MARKDOWN" << endl << endl << endl;

			cout << "Items currently in with a markdown:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM LowSaleItems LEFT OUTER JOIN Item ON LowSaleItems.SKU = Item.SKU");
			cout << endl;
			cout << "  Enter SKU of item with markdown to edit: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "SELECT * FROM LowSaleItems LEFT OUTER JOIN Item ON LowSaleItems.SKU = Item.SKU WHERE LowSaleItems.SKU = ";
			Input3.append(Input2);

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);

			Input1 = EditLowSaleItem(Input2);

			cout << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);
			cout << endl;

			cout << "   1. Edit Another Low Sale Item Markdown" << endl;
			cout << endl;
			cout << "   2. Low Sale Item Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "editlowsaleitem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "lowsaleitemmanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Low Sale Item Management: Delete Low Sale Item
		//
		while (DisplayStatus == "removelowsaleitem")
		{
			cout << endl;
			cout << "  LOW SALE ITEM MANAGEMENT: REMOVE LOW SALE ITEM MARKDOWN" << endl << endl << endl;

			cout << "Items currently in with a markdown:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM LowSaleItems LEFT OUTER JOIN Item ON LowSaleItems.SKU = Item.SKU");
			cout << endl;

			cout << "  Enter SKU of item with the markdown to remove: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "DELETE FROM LowSaleItems WHERE SKU = ";
			Input3.append(Input2);

			cout << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);


			cout << endl << endl;
			cout << "   1. Remove Another Low Sale Item Markdown" << endl;
			cout << endl;
			cout << "   2. Low Sale Item Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "removelowsaleitem";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "lowsaleitemmanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}




		// EMPLOYEE BELOW

		//
		// Employee Management
		//
		while (DisplayStatus == "employeemanagement")
		{
			cout << endl;
			cout << "  EMPLOYEE MANAGEMENT:" << endl << endl << endl;
			cout << "   1. View Employees" << endl;
			cout << "   2. Add Employee" << endl;
			cout << "   3. Edit Employee" << endl;
			cout << "   4. Delete Employee" << endl;
			cout << endl;
			cout << "   5. Main Menu" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "viewemployees";
			}
			else if (MainMenuInput == "2")
			{
				DisplayStatus = "addemployee";
			}
			else if (MainMenuInput == "3")
			{
				DisplayStatus = "editemployee";
			}
			else if (MainMenuInput == "4")
			{
				DisplayStatus = "deleteemployee";
			}
			else if (MainMenuInput == "5")
			{
				DisplayStatus = "mainmenu";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Employee Management: View Employees
		//
		while (DisplayStatus == "viewemployees")
		{
			cout << endl;
			cout << "  EMPLOYEE MANAGEMENT: VIEW EMPLOYEES" << endl << endl << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Employee");


			cout << endl;
			cout << "   1. Employee Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "employeemanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Employee Managment: Add Employee
		//
		while (DisplayStatus == "addemployee")
		{
			cout << endl;
			cout << "  EMPLOYEE MANAGEMENT: ADD EMPLOYEE" << endl << endl << endl;

			cout << "Current employees:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Employee");
			cout << endl << endl;

			Input1 = AddEmployee();
			cout << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);


			cout << endl << endl;
			cout << "   1. Add Another Employee" << endl;
			cout << endl;
			cout << "   2. Employee Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "addemployee";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "employeemanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Employee Management: Edit Employee
		//
		while (DisplayStatus == "editemployee")
		{
			cout << endl;
			cout << "  EMPLOYEE MANAGEMENT: EDIT EMPLOYEE" << endl << endl << endl;

			cout << "Current employees:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Employee");
			cout << endl << endl;


			cout << "  Enter EmployeeID of employee you wish to edit: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "SELECT * FROM Employee WHERE Employee.EmployeeID= ";
			Input3.append(Input2);

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);

			Input1 = EditEmployee(Input2);

			cout << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input1);
			cout << endl;

			cout << "   1. Edit Another Employee" << endl;
			cout << endl;
			cout << "   2. Employee Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "editemployee";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "employeemanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}
		//
		// Employee Management: Delete Employee
		//
		while (DisplayStatus == "deleteemployee")
		{
			cout << endl;
			cout << "  EMPLOYEE MANAGEMENT: DELETE EMPLOYEE" << endl << endl << endl;

			cout << "Current employees:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Employee");
			cout << endl << endl;


			cout << "  Enter EmployeeID of employee you wish to delete: >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "DELETE FROM Employee WHERE EmployeeID = ";
			Input3.append(Input2);

			cout << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);


			cout << endl << endl;
			cout << "   1. Delete Another Employee" << endl;
			cout << endl;
			cout << "   2. Employee Management" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "deleteemployee";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "employeemanagement";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}



		//
		// View Transaction Log
		//
		while (DisplayStatus == "viewtransactionlog")
		{
			cout << endl;
			cout << "  VIEW TRANSACTION LOG" << endl << endl << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT TransactionLog.OrderID, TransactionLog.EmployeeId, TransactionLog.SKU, TransactionLog.Quantity, Item.Description, TransactionLog.Price, TransactionLog.TimeOfOrder FROM TransactionLog LEFT OUTER JOIN Item ON TransactionLog.SKU = Item.SKU");


			cout << endl;
			cout << "   1. Main Menu" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "mainmenu";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}



		//
		// Create Transaction
		//
		while (DisplayStatus == "createtransaction")
		{
			// input1: EmployeeID
			// input2: OrderID
			// input3: SQL query

			cout << endl;
			cout << "  CREATE TRANSACTION" << endl << endl << endl;

			cout << "Current employees:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Employee");
			cout << endl << endl;


			cout << "  Please enter your EmployeeID: >> ";
			cin >> Input1;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "SELECT * FROM Employee WHERE Employee.EmployeeID= ";
			Input3.append(Input1);
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);

			cout << endl << endl;
			cout << "** To add items to a pre-existing order, choose an order below." << endl;
			cout << "** To add items to a new order, enter a new order number that is not shown below:" << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT DISTINCT OrderID FROM TransactionLog");
			cout << endl << endl;
			cout << "  Please enter your chosen OrderID (7 numerical digits): >> ";
			cin >> Input2;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			system("cls");
			DisplayStatus = "additemstoorder";

			/*
			cout << endl;
			cout << "   1. Main Menu" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
			DisplayStatus = "mainmenu";
			}

			else
			{
			WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
			cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
			WasValidResponse = true;
			}*/
		}
		//
		// Create Transaction: Add Items To Order
		//
		while (DisplayStatus == "additemstoorder")
		{


			// input1: EmployeeID
			// input2: OrderID
			// input3: SQL query

			// input4: SKU
			// input5: Quantity

			cout << endl;
			cout << "  CREATE TRANSACTION: ADD ITEMS TO THE ORDER" << endl << endl << endl;

			cout << "Current Order:" << endl;
			Input3 = "SELECT TransactionLog.OrderID, TransactionLog.EmployeeId, TransactionLog.SKU, TransactionLog.Quantity, Item.Description, TransactionLog.Price, TransactionLog.TimeOfOrder FROM TransactionLog LEFT OUTER JOIN Item ON TransactionLog.SKU = Item.SKU WHERE OrderID =";
			Input3.append(Input2);
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);

			cout << endl << endl;

			ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT Item.SKU, Item.Description, Item.Price, Inventory.OnHands FROM Inventory LEFT OUTER JOIN Item ON Inventory.SKU = Item.SKU");
			cout << endl << endl;

			cout << "  Please enter the SKU of an item you wish to add to the order: >> ";
			cin >> Input4;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			cout << "  Please enter the Quantity that you wish to add to the order: >> ";
			cin >> Input5;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			Input3 = "INSERT INTO TransactionLog VALUES ('";
			Input3.append(Input2);
			Input3.append("', '");
			Input3.append(Input1);
			Input3.append("', '");
			Input3.append(Input4);
			Input3.append("', '");
			Input3.append(Input5);

			//Input3.append("', (SELECT Item.Price WHERE Item.SKU = ");
			//Input3.append(Input4);
			//Input3.append("* ((100 - (SELECT LowSaleItems.MarkdownAmount WHERE LowSaleItems.SKU = ");
			//Input3.append(Input4);
			//Input3.append(")) DIV 100), CURRENT_TIMESTAMP");

			Input3.append("', ");
			Input3.append("(SELECT Price FROM Item WHERE Item.SKU = ");
			Input3.append(Input4);
			Input3.append(") * ");
			Input3.append(Input5);
			Input3.append("	, ");
			Input3.append("GETDATE()");
			Input3.append(")");

			cout << endl;
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);

			cout << endl << endl;

			cout << "Updated Order:" << endl;
			Input3 = "SELECT TransactionLog.OrderID, TransactionLog.EmployeeId, TransactionLog.SKU, TransactionLog.Quantity, Item.Description, TransactionLog.Price, TransactionLog.TimeOfOrder FROM TransactionLog LEFT OUTER JOIN Item ON TransactionLog.SKU = Item.SKU WHERE OrderID =";
			Input3.append(Input2);
			ExecuteSQLQuery(hEnv, hDbc, hStmt, Input3);



			cout << endl;
			cout << "   1. Add Another Item" << endl;
			cout << endl;
			cout << "   2. Return to Main Menu" << endl;
			cout << endl;

			cout << endl;
			cout << "  >> ";


			cin >> MainMenuInput;
			cin.clear();
			cin.ignore(INT_MAX, '\n');

			cout << endl << endl;

			if (MainMenuInput == "1")
			{
				DisplayStatus = "additemstoorder";
			}
			if (MainMenuInput == "2")
			{
				DisplayStatus = "mainmenu";
			}

			else
			{
				WasValidResponse = false;
			}


			system("cls");

			if (WasValidResponse == false)
			{
				cout << "ERROR: INVALID RESPONSE GIVEN" << endl;
				WasValidResponse = true;
			}
		}







		//DisplayStatus = "mainmenu"; // return to main menu
	}

	//	ExecuteSQLQuery(hEnv, hDbc, hStmt, "SELECT * FROM Item");

Exit:

	// Free ODBC handles and exit

	if (hStmt)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	}

	if (hDbc)
	{
		SQLDisconnect(hDbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	}

	if (hEnv)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
	}

	wprintf(L"\nDisconnected.");

	return 0;

}

/************************************************************************
/* DisplayResults: display results of a select query
/*
/* Parameters:
/*      hStmt      ODBC statement handle
/*      cCols      Count of columns
/************************************************************************/

void DisplayResults(HSTMT       hStmt,
	SQLSMALLINT cCols)
{
	BINDING         *pFirstBinding, *pThisBinding;
	SQLSMALLINT     cDisplaySize;
	RETCODE         RetCode = SQL_SUCCESS;
	int             iCount = 0;

	// Allocate memory for each column 

	AllocateBindings(hStmt, cCols, &pFirstBinding, &cDisplaySize);

	// Set the display mode and write the titles

	DisplayTitles(hStmt, cDisplaySize + 1, pFirstBinding);


	// Fetch and display the data

	bool fNoData = false;

	do {
		// Fetch a row

		if (iCount++ >= gHeight - 2)
		{
			int     nInputChar;
			bool    fEnterReceived = false;

			while (!fEnterReceived)
			{
				wprintf(L"              ");
				SetConsole(cDisplaySize + 2, TRUE);
				wprintf(L"   Press ENTER to continue, Q to quit (height:%hd)", gHeight);
				SetConsole(cDisplaySize + 2, FALSE);

				nInputChar = _getch();
				wprintf(L"\n");
				if ((nInputChar == 'Q') || (nInputChar == 'q'))
				{
					goto Exit;
				}
				else if ('\r' == nInputChar)
				{
					fEnterReceived = true;
				}
				// else loop back to display prompt again
			}

			iCount = 1;
			DisplayTitles(hStmt, cDisplaySize + 1, pFirstBinding);
		}

		TRYODBC(hStmt, SQL_HANDLE_STMT, RetCode = SQLFetch(hStmt));

		if (RetCode == SQL_NO_DATA_FOUND)
		{
			fNoData = true;
		}
		else
		{

			// Display the data.   Ignore truncations

			for (pThisBinding = pFirstBinding;
				pThisBinding;
				pThisBinding = pThisBinding->sNext)
			{
				if (pThisBinding->indPtr != SQL_NULL_DATA)
				{
					wprintf(pThisBinding->fChar ? DISPLAY_FORMAT_C : DISPLAY_FORMAT,
						PIPE,
						pThisBinding->cDisplaySize,
						pThisBinding->cDisplaySize,
						pThisBinding->wszBuffer);
				}
				else
				{
					wprintf(DISPLAY_FORMAT_C,
						PIPE,
						pThisBinding->cDisplaySize,
						pThisBinding->cDisplaySize,
						L"<NULL>");
				}
			}
			wprintf(L" %c\n", PIPE);
		}
	} while (!fNoData);

	SetConsole(cDisplaySize + 2, TRUE);
	wprintf(L"%*.*s", cDisplaySize + 2, cDisplaySize + 2, L" ");
	SetConsole(cDisplaySize + 2, FALSE);
	wprintf(L"\n");

Exit:
	// Clean up the allocated buffers

	while (pFirstBinding)
	{
		pThisBinding = pFirstBinding->sNext;
		free(pFirstBinding->wszBuffer);
		free(pFirstBinding);
		pFirstBinding = pThisBinding;
	}
}

/************************************************************************
/* AllocateBindings:  Get column information and allocate bindings
/* for each column.
/*
/* Parameters:
/*      hStmt      Statement handle
/*      cCols       Number of columns in the result set
/*      *lppBinding Binding pointer (returned)
/*      lpDisplay   Display size of one line
/************************************************************************/

void AllocateBindings(HSTMT         hStmt,
	SQLSMALLINT   cCols,
	BINDING       **ppBinding,
	SQLSMALLINT   *pDisplay)
{
	SQLSMALLINT     iCol;
	BINDING         *pThisBinding, *pLastBinding = NULL;
	SQLLEN          cchDisplay, ssType;
	SQLSMALLINT     cchColumnNameLength;

	*pDisplay = 0;

	for (iCol = 1; iCol <= cCols; iCol++)
	{
		pThisBinding = (BINDING *)(malloc(sizeof(BINDING)));
		if (!(pThisBinding))
		{
			fwprintf(stderr, L"Out of memory!\n");
			exit(-100);
		}

		if (iCol == 1)
		{
			*ppBinding = pThisBinding;
		}
		else
		{
			pLastBinding->sNext = pThisBinding;
		}
		pLastBinding = pThisBinding;


		// Figure out the display length of the column (we will
		// bind to char since we are only displaying data, in general
		// you should bind to the appropriate C type if you are going
		// to manipulate data since it is much faster...)

		TRYODBC(hStmt,
			SQL_HANDLE_STMT,
			SQLColAttribute(hStmt,
				iCol,
				SQL_DESC_DISPLAY_SIZE,
				NULL,
				0,
				NULL,
				&cchDisplay));


		// Figure out if this is a character or numeric column; this is
		// used to determine if we want to display the data left- or right-
		// aligned.

		// SQL_DESC_CONCISE_TYPE maps to the 1.x SQL_COLUMN_TYPE. 
		// This is what you must use if you want to work
		// against a 2.x driver.

		TRYODBC(hStmt,
			SQL_HANDLE_STMT,
			SQLColAttribute(hStmt,
				iCol,
				SQL_DESC_CONCISE_TYPE,
				NULL,
				0,
				NULL,
				&ssType));


		pThisBinding->fChar = (ssType == SQL_CHAR ||
			ssType == SQL_VARCHAR ||
			ssType == SQL_LONGVARCHAR);

		pThisBinding->sNext = NULL;

		// Arbitrary limit on display size
		if (cchDisplay > DISPLAY_MAX)
			cchDisplay = DISPLAY_MAX;

		// Allocate a buffer big enough to hold the text representation
		// of the data.  Add one character for the null terminator

		pThisBinding->wszBuffer = (WCHAR *)malloc((cchDisplay + 1) * sizeof(WCHAR));

		if (!(pThisBinding->wszBuffer))
		{
			fwprintf(stderr, L"Out of memory!\n");
			exit(-100);
		}

		// Map this buffer to the driver's buffer.   At Fetch time,
		// the driver will fill in this data.  Note that the size is 
		// count of bytes (for Unicode).  All ODBC functions that take
		// SQLPOINTER use count of bytes; all functions that take only
		// strings use count of characters.

		TRYODBC(hStmt,
			SQL_HANDLE_STMT,
			SQLBindCol(hStmt,
				iCol,
				SQL_C_TCHAR,
				(SQLPOINTER)pThisBinding->wszBuffer,
				(cchDisplay + 1) * sizeof(WCHAR),
				&pThisBinding->indPtr));


		// Now set the display size that we will use to display
		// the data.   Figure out the length of the column name

		TRYODBC(hStmt,
			SQL_HANDLE_STMT,
			SQLColAttribute(hStmt,
				iCol,
				SQL_DESC_NAME,
				NULL,
				0,
				&cchColumnNameLength,
				NULL));

		pThisBinding->cDisplaySize = max((SQLSMALLINT)cchDisplay, cchColumnNameLength);
		if (pThisBinding->cDisplaySize < NULL_SIZE)
			pThisBinding->cDisplaySize = NULL_SIZE;

		*pDisplay += pThisBinding->cDisplaySize + DISPLAY_FORMAT_EXTRA;

	}

	return;

Exit:

	exit(-1);

	return;
}


/************************************************************************
/* DisplayTitles: print the titles of all the columns and set the
/*                shell window's width
/*
/* Parameters:
/*      hStmt          Statement handle
/*      cDisplaySize   Total display size
/*      pBinding        list of binding information
/************************************************************************/

void DisplayTitles(HSTMT     hStmt,
	DWORD     cDisplaySize,
	BINDING   *pBinding)
{
	WCHAR           wszTitle[DISPLAY_MAX];
	SQLSMALLINT     iCol = 1;

	SetConsole(cDisplaySize + 2, TRUE);

	for (; pBinding; pBinding = pBinding->sNext)
	{
		TRYODBC(hStmt,
			SQL_HANDLE_STMT,
			SQLColAttribute(hStmt,
				iCol++,
				SQL_DESC_NAME,
				wszTitle,
				sizeof(wszTitle), // Note count of bytes!
				NULL,
				NULL));

		wprintf(DISPLAY_FORMAT_C,
			PIPE,
			pBinding->cDisplaySize,
			pBinding->cDisplaySize,
			wszTitle);
	}

Exit:

	wprintf(L" %c", PIPE);
	SetConsole(cDisplaySize + 2, FALSE);
	wprintf(L"\n");

}


/************************************************************************
/* SetConsole: sets console display size and video mode
/*
/*  Parameters
/*      siDisplaySize   Console display size
/*      fInvert         Invert video?
/************************************************************************/

void SetConsole(DWORD dwDisplaySize,
	BOOL  fInvert)
{
	HANDLE                          hConsole;
	CONSOLE_SCREEN_BUFFER_INFO      csbInfo;

	// Reset the console screen buffer size if necessary

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if (hConsole != INVALID_HANDLE_VALUE)
	{
		if (GetConsoleScreenBufferInfo(hConsole, &csbInfo))
		{
			if (csbInfo.dwSize.X <  (SHORT)dwDisplaySize)
			{
				csbInfo.dwSize.X = (SHORT)dwDisplaySize;
				SetConsoleScreenBufferSize(hConsole, csbInfo.dwSize);
			}

			gHeight = csbInfo.dwSize.Y;
		}

		if (fInvert)
		{
			SetConsoleTextAttribute(hConsole, (WORD)(csbInfo.wAttributes | BACKGROUND_BLUE));
		}
		else
		{
			SetConsoleTextAttribute(hConsole, (WORD)(csbInfo.wAttributes & ~(BACKGROUND_BLUE)));
		}
	}
}


/************************************************************************
/* HandleDiagnosticRecord : display error/warning information
/*
/* Parameters:
/*      hHandle     ODBC handle
/*      hType       Type of handle (HANDLE_STMT, HANDLE_ENV, HANDLE_DBC)
/*      RetCode     Return code of failing command
/************************************************************************/

void HandleDiagnosticRecord(SQLHANDLE      hHandle,
	SQLSMALLINT    hType,
	RETCODE        RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];


	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}

	while (SQLGetDiagRec(hType,
		hHandle,
		++iRec,
		wszState,
		&iError,
		wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)),
		(SQLSMALLINT *)NULL) == SQL_SUCCESS)
	{
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5))
		{
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}

}
