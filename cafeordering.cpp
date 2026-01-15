#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <iomanip>

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

#define RESET   "\033[0m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"
#define BLUE    "\033[34m"

using namespace std;

// For Demonstration only. Never save your password in the code!
const string server = "tcp://127.0.0.1:3306";
const string username = "root";
const string password = "";

// Function Prototype User Login and Management 
/* Staff & Admin */ bool login(sql::Connection* con, int id, string pass, string position);
/* Admin */ void deleteStaff(sql::Connection* con, int id);
/* Admin */ void searchStaff(sql::Connection* con, int id);
/* Admin */ void addStaff(sql::Connection* con, string name, string pass, string position);
/* Admin */ void viewStaffList(sql::Connection* con);

// Function Prototype Ordering 
/* Staff & Admin */ void displayMenu(sql::Connection* con);
/* Admin */ void updateMenuAvb(sql::Connection* con);
/* Admin */ void addMenu(sql::Connection* con);
/* Admin */ void deleteMenu(sql::Connection* con);
/* Staff */ void createOrder(sql::Connection* con, int id);

// Function Prototype Payment and Sales Reporting
// Payment:
/* Staff */ bool checkMembership(sql::Connection* con, int& memberID);
/* Staff */ void registerMembership(sql::Connection* con, int& memberID);
/* Staff */ double cashRounding(double amount);
/* Staff */ void payment(sql::Connection* con, int orderID);

// Sales Reporting:
/* Admin */ void salesReport(sql::Connection* con, string reportDate, int adminID, int& outsalesID);
/* Admin */ void orderTrend(sql::Connection* con, string date, int salesID, int adminID);
/* Admin */ void summaryReport(sql::Connection* con, string reportDate, int adminID, int salesID);

// For Hidden Password Setup
string hiddenPass() {
	char hPass;
	string pass = "";

	while (true) {
		hPass = _getch();

		if (hPass == 13) {
			cout << endl;
			break;
		}
		else if (hPass == 8) {
			if (!pass.empty())
				cout << "\b \b";
			pass.pop_back();
		}
		else {
			pass.push_back(hPass);
			cout << "*";
		}
	}
	return pass;
}
struct OrderItem {
	int CatID = 0 ;
	string menuName =" ";
	string drinkType = " ";
	int qty = 0;
	double price = 0.0;
	double subtotal =0.0;
};

// Main Function
int main() {

#ifdef _WIN32
	system("");
#endif

	sql::Driver* driver;
	sql::Connection* con;

	try {
		driver = get_driver_instance();
		con = driver->connect(server, username, password);
		con->setSchema("cafeorderingdb");
		cout << GREEN << "Database connection successful!" << RESET << endl;
	}
	catch (sql::SQLException& e) {
		cout << RED <<  "Could not connect to server. Error message: " << e.what() << RESET << endl;
		system("pause");
		exit(1);
	}

	string choice, staffPass, adminPass;
	int staffID, adminID;

	while (true) {
		cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << CYAN << "      Welcome to Morning Nest Cafe!" << RESET << endl;
		cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << CYAN << "\n\t1- Staff\t2- Admin" << RESET << endl;
		cout << "\nSelect role: ";
		getline(cin, choice);
		
		// Staff Login
		if (choice == "1")
		{
			bool staffLoggedIn = false;

			while (!staffLoggedIn) {
				cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
				cout << CYAN <<  "           Morning Nest Cafe" << RESET << endl;
				cout << BLUE << "              Staffs Login" << RESET <<  endl;
				cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
				cout << "\nStaff ID: ";
				cin >> staffID;

				bool validID = false;

				try {
					sql::PreparedStatement* pstmt = con->prepareStatement("SELECT StaffName FROM staff WHERE StaffID = ?");
					pstmt->setInt(1, staffID);
					sql::ResultSet* res = pstmt->executeQuery();

					if (res->next()) {
						string name = res->getString("StaffName");
						cout << CYAN << "\nWelcome, " << name << CYAN << "!" << RESET << endl;
						validID = true;
					}
					else {
						cout << RED << "\n[!] Invalid ID!\n" << RESET << endl;
					}
					delete res;
					delete pstmt;
				}
				catch (sql::SQLException& e) {
					cout << RED << "\n[!]Error retrieving users: " << e.what() << RESET << endl;
					delete con;
					return 0;
				}

				if (!validID) {
					continue; 
				}

				cout << "\nPassword: ";
				staffPass = hiddenPass();

				if (login(con, staffID, staffPass, "staff")) {
					cout << GREEN << "\nLogin successful!" << RESET << endl;
					staffLoggedIn = true;

					bool staffMenuActive = true;
					while (staffMenuActive) {
						cout << "\nCreate order (" << GREEN << "Yes" << RESET << "/" << RED << "No" << RESET "): ";
						cin.ignore();
						getline(cin, choice);

						if (choice == "Y" || choice == "y") {
							createOrder(con, staffID);
							staffMenuActive = false; 
						}
						else if (choice == "N" || choice == "n") {
							cout << "\nReturning to Main menu...\n" << endl;
							staffMenuActive = false;
						}
						else {
							cout << RED << "[!] Invalid input. " << RESET << endl;
							cout << "Please enter Yes or No.\n";
						}
					}
				}
				else {
					cout << RED << "\n[!] Login failed." << RESET << endl;
					cout << "Returning to Main menu..." << endl;
				}

				break; 
			}
		}

		// Admin Login
		else if (choice == "2") {

			cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
			cout << CYAN << "              Morning Nest Cafe" << RESET << endl;
			cout << BLUE << "                 Admin Login" << RESET << endl;
			cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
			cout << "\nAdmin ID: ";
			cin >> adminID;

			bool validID = false;

			try {
				sql::PreparedStatement* pstmt = con->prepareStatement("SELECT AdminName FROM admin WHERE AdminID = ?");
				pstmt->setInt(1, adminID);
				sql::ResultSet* res = pstmt->executeQuery();

				if (res->next()) {
					string name = res->getString("AdminName");
					cout << CYAN << "\nWelcome, " << name << CYAN << "!\n" << RESET;
					validID = true;
				}
				else {
					cout << RED << "[!] Invalid ID!" << RESET << endl;
				}
				delete res;
				delete pstmt;
			}
			catch (sql::SQLException& e) {
				cout << RED << "[!] Error retrieving users: " << e.what() << RESET << endl;
				delete con;
				return 0;
			}

			if (!validID) {
				continue; 
			}

			cout << "\nPassword: ";
			adminPass = hiddenPass();

			if (login(con, adminID, adminPass, "admin")) {
				cout << GREEN << "\nLogin successful!" << RESET << endl;

				bool adminMenuActive = true;
				while (adminMenuActive) {
					cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
					cout << CYAN << "               Morning Nest Cafe" << RESET << endl;
					cout << BLUE << "           Admin Management System" << RESET << endl;
					cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
					cout << CYAN << "\n1- Manage Staff" << endl;
					cout << "2- Update Menu" << endl;
					cout << "3- Sales Reporting" << endl;
					cout << "0- Logout" << RESET << endl;
					cout << "\nEnter choice: ";
					cin.ignore();
					getline(cin, choice);

					if (choice == "1") {
						int adminChoice;
						do {
							cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
							cout << BLUE << "               Staff Management" << RESET << endl;
							cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
							cout << CYAN << "\n1- Add New Staff" << endl;
							cout << "2- Delete Staff" << endl;
							cout << "3- Search Staff" << endl;
							cout << "4- View Staff" << endl;
							cout << "0- Back" << RESET << endl;
							cout << "\nEnter choice: ";
							cin >> adminChoice;
							cin.ignore();

							if (adminChoice == 1) {
								string name, pass, position;
								cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
								cout << BLUE << "               Staff Management" << RESET <<  endl;
								cout << CYAN << "                  Add Staff" << RESET << endl;
								cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
								cout << "\nEnter staff name: ";
								getline(cin, name);

								cout << "Enter staff password: ";
								pass = hiddenPass();

								cout << "Enter staff position: ";
								getline(cin, position);

								addStaff(con, name, pass, position);
							}
							else if (adminChoice == 2) {
								int id;
								cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
								cout << BLUE << "               Staff Management" << RESET << endl;
								cout << CYAN << "                 Delete Staff" << RESET << endl;
								cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
								cout << "\nEnter staff ID to delete: ";
								cin >> id;
								cin.ignore();

								deleteStaff(con, id);
							}
							else if (adminChoice == 3) {
								int id;
								cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
								cout << BLUE << "               Staff Management" << RESET << endl;
								cout << CYAN << "                 Search Staff" << RESET << endl;
								cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
								cout << "\nEnter staff ID to search: ";
								cin >> id;
								cin.ignore();
								searchStaff(con, id);
							}
							else if (adminChoice == 4) {
								viewStaffList(con);
							}
							else if (adminChoice == 0) {
								cout << "\nReturning to Admin menu...\n";
							}
							else {
								cout << RED << "\n[!] Invalid option" << RESET << endl;
							}
						} while (adminChoice != 0);
					}
					else if (choice == "2") {
						int menuChoice;

						do {
							cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
							cout << BLUE << "               Menu Management" << RESET << endl;
							cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
							cout << CYAN << "\n1- Display Menu" << endl;
							cout << "2- Update Availability" << endl;
							cout << "3- Add Menu" << endl;
							cout << "4- Remove Menu" << endl;
							cout << "0- Back" << RESET << endl;
							cout << "\nEnter choice: ";
							cin >> menuChoice;

							if (menuChoice == 1) {
								displayMenu(con);
							}
							else if (menuChoice == 2) {
								updateMenuAvb(con);
							}
							else if (menuChoice == 3) {
								addMenu(con);
							}
							else if (menuChoice == 4) {
								deleteMenu(con);
							}
							else if (menuChoice == 0) {
								cout << "\nReturning to Admin menu...\n";
							}
							else {
								cout << RED << "\n[!] Invalid option." << RESET << endl;
							}

						} while (menuChoice != 0);
					}
					else if (choice == "3") {

						int adminChoice;
						string reportDate;
						int salesID = -1;

						static string lastReportDate;

						do {
							cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
							cout << BLUE << "           Sales Reporting Management" << RESET << endl;
							cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
							cout << CYAN << "\n1- Sales Report" << endl;
							cout << "2- Order Trend Report" << endl;
							cout << "3- Summary Report" << endl;
							cout << "0- Back" << RESET << endl;
							cout << "\nEnter choice: ";
							cin >> adminChoice;

							if (adminChoice == 1) {

								try {
									sql::Statement* stmt = con->createStatement();
									sql::ResultSet* rs = stmt->executeQuery(
										"SELECT DISTINCT DATE(PayTime) AS PayDate "
										"FROM payment "
										"ORDER BY PayDate DESC "
										"LIMIT 5"
									);

									bool hasDates = false;
									cout << "\n" << GREEN << "Available dates with payment data:" << RESET << endl;
									cout << "---------------------------------------------\n";

									while (rs->next()) {
										hasDates = true;
										cout << "  - " << rs->getString("PayDate") << endl;

										
										if (lastReportDate.empty()) {
											lastReportDate = rs->getString("PayDate");
										}
									}

									if (!hasDates) {
										cout << RED << "\n[!] No payment data available in database." << RESET << endl;
										delete rs;
										delete stmt;
										continue;
									}

									cout << "---------------------------------------------\n";
									delete rs;
									delete stmt;
								}
								catch (sql::SQLException& e) {
									cout << RED << "\n[!] Error checking available dates: " << e.what() << RESET << endl;
									continue;
								}
								cout << "\nEnter report date (YYYY-MM-DD): ";
								cin >> reportDate;

								
								if (reportDate.length() != 10 || reportDate[4] != '-' || reportDate[7] != '-') {
									cout << RED << "\n[!] Invalid date format! Use YYYY-MM-DD (e.g: 2026-01-01)" << RESET << endl;
									continue;
								}

								try {
									sql::PreparedStatement* pstmt = con->prepareStatement(
										"SELECT COUNT(*) AS count FROM payment WHERE DATE(PayTime) = ?"
									);
									pstmt->setString(1, reportDate);
									sql::ResultSet* rs = pstmt->executeQuery();

									int count = 0;
									if (rs->next()) {
										count = rs->getInt("count");
									}

									delete rs;
									delete pstmt;

									if (count == 0) {
										cout << RED << "\n[!] No payment data found for date: " << reportDate << RESET << endl;
										cout << "Please select a date from the available dates shown above.\n";
										continue; 
									}

									cout << CYAN << "\nFound " << count << " payment(s) for date: " << reportDate << RESET << endl;
								}
								catch (sql::SQLException& e) {
									cout << RED << "\n[!] Error checking date: " << e.what() << RESET << endl;
									continue;
								}

								lastReportDate = reportDate;

								salesID = -1;

								cout << "\n" << CYAN << "Generating Sales Report for " << reportDate << "..." << RESET << endl;
								salesReport(con, reportDate, adminID, salesID);

								if (salesID == -1) {
									cout << RED << "\n[!] Sales Report generation failed." << RESET << endl;
								}
								else {
									cout << GREEN << "\nSales Report generated successfully! (SalesID: "
										<< salesID << ")" << RESET << endl;
									cout << "\nYou can now proceed to Order Trend Report (Option 2).\n";
								}
							}

							else if (adminChoice == 2) {

								if (salesID == -1) {
									cout << RED << "\n[!] Please generate Sales Report (Option 1) first." << RESET << endl;
								}
								else {
									cout << "\n" << CYAN << "Generating Order Trend Report for "
										<< lastReportDate << "..." << RESET << endl;

									orderTrend(con, lastReportDate, salesID, adminID);

									cout << GREEN << "\nOrder Trend Report generated successfully!" << RESET << endl;
									cout << "\nYou can now proceed to Summary Report (Option 3).\n";
								}
							}

							else if (adminChoice == 3) {

								if (salesID == -1) {
									cout << RED << "\n[!] Please complete steps 1 & 2 first:" << RESET << endl;
									cout << CYAN << "  \n1. Generate Sales Report\n";
									cout << "2. Generate Order Trend Report\n";
									cout << "3. Then you can generate Summary Report\n" << RESET;
								}
								else {
									cout << "\n" << CYAN << "Generating Summary Report for "
										<< lastReportDate << "..." << RESET << endl;

									summaryReport(con, lastReportDate, adminID, salesID);

									cout << GREEN << "\nSummary Report completed!" << RESET << endl;

									
									char continueChoice;
									cout << "\nGenerate reports for another date? (" << GREEN << "Yes" << RESET << "/" << RED << "No" << RESET "): ";
									cin >> continueChoice;

									if (continueChoice == 'Y' || continueChoice == 'y') {
										salesID = -1; 
										lastReportDate = ""; 
										cout << "Report session reset. Please select Option 1 to start.\n" << endl;
									}
								}
							}

							else if (adminChoice == 0) {
								cout << "\nReturning to Admin menu...\n";
							}
							else {
								cout << RED << "\n[!] Invalid option." << RESET << endl;
							}

						} while (adminChoice != 0);
					}
					else if (choice == "0") {
						cout << "\nLogging out...\n";
						adminMenuActive = false; 
					}
					else {
						cout << RED << "\n[!] Invalid option." << RESET << endl;
						cout << "Please try again.\n";
					}
				}
			}
			else {
				cout << RED << "\n[!] Login failed." << RESET << endl;
				cout << "\nReturning to main menu...\n";
			}
		}
		else {
			cout << RED << "\n[!] Invalid role selected!" << RESET << endl;
			cout << "Please select 1 for Staff or 2 for Admin.\n";
		}
	}

	system("pause");
	delete con;
	return 0;
}

// User Login and Management

// Login for Staff and Admin
bool login(sql::Connection* con, int id, string pass, string position) {
	try {
		sql::PreparedStatement*
			pstmt = nullptr;
		sql::ResultSet*
			res = nullptr;

		if (position == "staff") {
			pstmt = con->prepareStatement("SELECT * FROM staff WHERE StaffID = ? AND StaffPass = ?");
		}
		else if (position == "admin") {
			pstmt = con->prepareStatement("SELECT * FROM admin WHERE AdminID =? AND AdminPass = ?");
		}
		else {
			return false;
		}

		pstmt->setInt(1, id);
		pstmt->setString(2, pass);
		res = pstmt->executeQuery();

		bool success = res->next();
		delete res;
		delete pstmt;
		return success;
	}
	catch (sql::SQLException& e) {
		cout << RED << "\n[!] Error retrieving users: " << e.what() << RESET << endl;
		return false;
	}
}

// (Admin) Staff  Management: Delete/Remove Staff
void deleteStaff(sql::Connection* con, int id) {
	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement("SELECT StaffName, StaffPosition FROM staff WHERE StaffID = ?");
		pstmt->setInt(1, id);
		sql::ResultSet* res = pstmt->executeQuery();

		if (!res->next()) {
			cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
			cout << BLUE << "                   Staff Management" << RESET << endl;
			cout << CYAN << "                     Delete Staff" << RESET << endl;
			cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
			cout << RED << "\n[!] No staff found with Staff ID: " << id << RESET << endl;
			delete res;
			delete pstmt;
			return;
		}

		string name = res->getString("StaffName");
		string position = res->getString("StaffPosition");

		delete res;
		delete pstmt;

		cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << BLUE << "                Staff Management" << RESET << endl;
		cout << CYAN << "                   Delete Staff" << RESET << endl;
		cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << GREEN << "\nStaff Found: " << RESET << endl;
		cout << CYAN << "\nID       : " << id << endl;
		cout << "Name     : " << name << endl;
		cout << "Position : " << position << RESET << endl;


		char confirm;
		cout << "\nAre you sure you want to delete this staff?" << endl;
		cout << "(" << GREEN << "Yes" << RESET << " / " << RED << "No" << RESET ") : ";
		cin >> confirm;

		if (confirm != 'Y' && confirm != 'y') {
			cout << YELLOW << "\nDelete cancelled.\n" << RESET;
			return;
		}

		pstmt = con->prepareStatement("DELETE FROM staff WHERE StaffID = ?");
		pstmt->setInt(1, id);
		pstmt->executeUpdate();
		delete pstmt;

		cout << GREEN << "\nStaff \"" << name << "\" has been successfully removed." << RESET << endl;
	}
	catch (sql::SQLException& e) {
		cout << RED << "\n[!] Error deleting staff: " << e.what() << RESET << endl;
	}
}


// (Admin) Staff Management: Add Staff
void addStaff(sql::Connection* con, string name, string pass, string position) {
	try {
		sql::PreparedStatement* pstmt = con->prepareStatement(
			"INSERT INTO staff (StaffName, StaffPass, StaffPosition) VALUES (?, ?, ?)"
		);
		pstmt->setString(1, name);
		pstmt->setString(2, pass);
		pstmt->setString(3, position);
		pstmt->executeUpdate();
		delete pstmt;

		sql::Statement* stmt = con->createStatement();
		sql::ResultSet* res = stmt->executeQuery("SELECT LAST_INSERT_ID() AS last_id");
		int newID = 0;
		if (res->next()) newID = res->getInt("last_id");

		cout << GREEN << "\nStaff added successfully!" << RESET << endl;
		cout << CYAN << "\nStaff ID : " << newID << "\n";
		cout << "Name     : " << name << "\n";
		cout << "Position : " << position << "\n" << RESET;

		delete res;
		delete stmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "\n[!] Error adding new staff: " << e.what() << RESET << endl;
	}
}

// (Admin) Staff Management: Search Staff
void searchStaff(sql::Connection* con, int id) {
	try {
		sql::PreparedStatement* pstmt;
		sql::ResultSet* res;
		pstmt = con->prepareStatement("SELECT * FROM staff WHERE StaffID =?");
		pstmt->setInt(1, id);
		res = pstmt->executeQuery();

		if (res->next()) {
			cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
			cout << BLUE << "               Staff Management" << RESET << endl;
			cout << CYAN << "                   Search Staff" << RESET << endl;
			cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
			cout << CYAN << "\nStaff ID: " << res->getInt("StaffID") << "\nName: " << res->getString("StaffName") 
				 << "\nPosition: " << res->getString("StaffPosition") << RESET << endl;
		}
		else {
			cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
			cout << BLUE <<"               Staff Management" << RESET << endl;
			cout << CYAN << "                Search Staff" << RESET << endl;
			cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
			cout << RED << "\n[!] No Staff is found with Staff ID: " << id << RESET << endl;
		}

		delete res;
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << BLUE << "               Staff Management" << RESET << endl;
		cout << CYAN << "                 Search Staff" << RESET << endl;
		cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << RED << "\n[!] Error retrieving user: " << e.what() << RESET << endl;
	}
}

// (Admin) Staff Management: View All Staff List
void viewStaffList(sql::Connection* con) {
	try {
		sql::Statement* stmt = con->createStatement();
		sql::ResultSet* res = stmt->executeQuery("SELECT StaffID, StaffName, StaffPosition from staff");
		cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << BLUE << "                   Staff Management" << RESET << endl;
		cout << CYAN << "                      Staff List" << RESET << endl;
		cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n" << endl;

		cout << YELLOW << string(55, '-') << RESET << endl;
		cout << left << setw(10) << "Staff ID" << setw(25) << "Name" << setw(20) << "Position" << endl;
		cout << YELLOW << string(55, '-') << RESET << endl;

		bool found = false;
		while (res->next()) {
			found = true;
			cout << left << setw(10) << res->getInt("StaffID") << setw(25) << res->getString("StaffName") 
				 << setw(20) << setw(20) << res->getString("StaffPosition") << endl;
		}

		if (!found) {
			cout << RED << "\n[!] No staff are registered in the system." << RESET << endl;
		}

		delete res;
		delete stmt;

		cout << YELLOW << string(55, '-') << RESET << endl;
	}
	catch (sql::SQLException& e) {
		cout << RED << "[!] Error displaying staff list: " << e.what() << RESET << endl;
	}
}

// Ordering Module

// (Staff/Admin) Ordering Module: Display Menu
void displayMenu(sql::Connection* con)
{
	try {
		sql::Statement* stmt = con->createStatement();
		sql::ResultSet* res = stmt->executeQuery(
			"SELECT CatID, CatName, MenuName, PriceHot_RM, PriceCold_RM, Price_RM, Availability FROM menu");

		cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << CYAN << "                                     Morning Nest Cafe" << RESET << endl;
		cout << BLUE << "                                           Menu" << RESET << endl;
		cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n" << endl;

		cout << YELLOW << string(90, '-') << RESET << endl;
		cout << left << setw(5) << "ID" << setw(15) << "Category" << setw(25) << "Item" << setw(10) << "Hot(RM)" << setw(10)
			<< "Cold(RM)" << setw(10) << "Food(RM)" << setw(10) << "Availability" << RESET << endl;
		cout << YELLOW << string(90, '-') << RESET << endl;

		
		ios_base::fmtflags originalFlags = cout.flags();
		streamsize originalPrecision = cout.precision();

		while (res->next()) {
			int id = res->getInt("CatID");
			string catName = res->getString("CatName");
			string menuName = res->getString("MenuName");
			string avb = res->getString("Availability");

			cout << left << setw(5) << id
				<< setw(15) << catName
				<< setw(25) << menuName;

			if (res->isNull("PriceHot_RM")) {
				cout << YELLOW << setw(10) << "-" << RESET;
			}
			else {
				cout << setw(10) << fixed << setprecision(2) << res->getDouble("PriceHot_RM");
			}

			if (res->isNull("PriceCold_RM")) {
				cout << YELLOW << setw(10) << "-" << RESET;
			}
			else {
				cout << setw(10) << fixed << setprecision(2) << res->getDouble("PriceCold_RM");
			}

			// Food Price
			if (res->isNull("Price_RM")) {
				cout << YELLOW << setw(10) << "-" << RESET;
			}
			else {
				cout << setw(10) << fixed << setprecision(2) << res->getDouble("Price_RM");
			}

			cout << setw(10) << avb << endl;
		}

		cout.flags(originalFlags);
		cout.precision(originalPrecision);

		cout << YELLOW << string(90, '-') << RESET << endl;
		delete res;
		delete stmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "\n[!] Error loading menu: " << e.what() << RESET << endl;
	}
}

// (Admin) Ordering Module: Update Menu Availability
void updateMenuAvb(sql::Connection* con) {

	int id;
	string status;

	cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	cout << BLUE << "               Menu Management" << RESET << endl;
	cout << CYAN << "           Update Menu Availability" << RESET << endl;
	cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	cout << "\nEnter menu CatID: ";
	cin >> id;

	cout << "Set availibility (" << GREEN << "Yes" << RESET << "/" << RED << "No" << RESET "): ";
	cin.ignore();
	getline(cin, status);

	if (status == "Yes" || status == "yes" || status == "1")
		status = "Yes";
	else if (status == "No" || status == "no" || status == "0")
		status = "No";
	else {
		cout << RED << "\n[!] Update failed! Invalid input. Please choose 'Yes' or 'No' only." << RESET << endl;
	}

	try {
		sql::PreparedStatement* pstmt = con->prepareStatement("UPDATE menu SET AVAILABILITY = ?  WHERE CatID = ?");

		pstmt->setString(1, status);
		pstmt->setInt(2, id);

		int rows = pstmt->executeUpdate();

		if (rows > 0) {
			cout << GREEN << "\nMenu availability update successfully!" << RESET << endl;
		}
		else {
			cout << RED << "\n[!] No menu found with ID: " << id << "." << RESET << endl;
		}

		delete pstmt;

	}
	catch (sql::SQLException& e) {
		cout << RED << "\n[!] Error updating menu: " << e.what() << RESET << endl;
	}
}

// (Admin): Ordering Module: Add Menu
void addMenu(sql::Connection* con) {

	string catName, menuName, avb;
	double priceHot, priceCold, foodPrice;
	double* ptrHot = nullptr;
	double* ptrCold = nullptr;
	double* ptrFood = nullptr;

	cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	cout << BLUE << "               Menu Management" << RESET << endl;
	cout << CYAN << "                  Add Menu" << RESET << endl;
	cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	cout << "\nEnter Category Name: ";
	cin.ignore();
	getline(cin >> ws, catName);
	cout << "Enter Menu Name: ";
	cin >> menuName;

	cout << "Enter Price of Hot Drink (0 if none): ";
	cin >> priceHot;
	if (priceHot != 0) ptrHot = &priceHot;

	cout << "Enter Price of Cold Drink (0 if none): ";
	cin >> priceCold;
	if (priceCold != 0) ptrCold = &priceCold;

	cout << "Enter Price of Food (0 if none): ";
	cin >> foodPrice;
	if (foodPrice != 0) ptrFood = &foodPrice;

	cout << "Availability (Yes/No): ";
	cin.ignore();
	getline(cin, avb);

	try {
		sql::PreparedStatement* pstmt = con->prepareStatement(
			"INSERT INTO menu (CatName, MenuName, PriceHot_RM, PriceCold_RM, Price_RM, Availability) "
			"VALUES (?, ?, ?, ?, ?, ?)"
		);

		pstmt->setString(1, catName);
		pstmt->setString(2, menuName);

		if (ptrHot) pstmt->setDouble(3, *ptrHot);
		else pstmt->setNull(3, sql::DataType::DOUBLE);

		if (ptrCold) pstmt->setDouble(4, *ptrCold);
		else pstmt->setNull(4, sql::DataType::DOUBLE);

		if (ptrFood) pstmt->setDouble(5, *ptrFood);
		else pstmt->setNull(5, sql::DataType::DOUBLE);

		pstmt->setString(6, avb);

		pstmt->executeUpdate();

		sql::Statement* stmt = con->createStatement();
		sql::ResultSet* res = stmt->executeQuery("SELECT LAST_INSERT_ID() AS last_id");
		int newID = 0;
		if (res->next()) newID = res->getInt("last_id");

		cout << GREEN << "\nMenu added successfully!" << RESET << endl;
		cout << "\nID           : " << newID << "\n";
		cout << "Category     : " << catName << "\n";
		cout << "Menu         : " << menuName << "\n";
		cout << "Hot(RM)      : " << (ptrHot ? to_string(*ptrHot) : "-") << "\n";
		cout << "Cold(RM)     : " << (ptrCold ? to_string(*ptrCold) : "-") << "\n";
		cout << "Price(RM)    : " << (ptrFood ? to_string(*ptrFood) : "-") << "\n";
		cout << "Availability : " << avb << "\n";

		delete res;
		delete stmt;
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "\n[!] Error adding menu: " << e.what() << RESET << endl;
	}
}


// (Admin): Ordering Module: Remove Menu
void deleteMenu(sql::Connection* con) {

	int id;

	cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	cout << BLUE << "               Menu Management" << RESET << endl;
	cout << CYAN << "                Remove Menu" << RESET << endl;
	cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	cout << "\nEnter CatID to delete: ";
	cin >> id;

	try {
		sql::PreparedStatement* pstmt;
		pstmt = con->prepareStatement("DELETE FROM menu WHERE CatID = ?");
		pstmt->setInt(1, id);
		pstmt->executeUpdate();

		cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << BLUE << "               Menu Management" << RESET << endl;
		cout << CYAN << "                Remove Menu" << RESET << endl;
		cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << GREEN << "\nMenu successfully deleted." << RESET << endl;

		delete pstmt;
	}

	catch (sql::SQLException& e) {
		cout << RED << "\n[!] Error deleting the menu: " << e.what() << RESET << endl;
	}
}


// (Staff): Ordering Module: Create Order
void createOrder(sql::Connection* con, int staffID) {

	bool orderComplete = false;

	while (!orderComplete) {
		string custName, orderSource;
		int orderID = 0;

		cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
		cout << CYAN << "            Morning Nest Cafe" << RESET << endl;
		cout << BLUE << "                 Orders" << RESET << endl;
		cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
		cout << "\nCustomer Name: ";
		getline(cin >> ws, custName);
		cout << "Order Source:" << CYAN << "\t1 - WK\t\t2 - FD\n" << RESET;
		cout << "Select: ";
		getline(cin, orderSource);

		if (orderSource == "1") orderSource = "WK";
		else if (orderSource == "2") orderSource = "FD";
		else {
			cout << RED << "\n[!] Invalid order source." << RESET << endl;
			continue;
		}

		try {
			sql::PreparedStatement* pstmt = con->prepareStatement(
				"INSERT INTO orders (CustomerName, StaffID, OrderStatus, OrderTime, OrderSource, TotalPrice_RM, OrderHistory) "
				"VALUES (?, ?, 'Preparing', NOW(), ?, 0.00, 'Active')");

			pstmt->setString(1, custName);
			pstmt->setInt(2, staffID);
			pstmt->setString(3, orderSource);
			pstmt->executeUpdate();
			delete pstmt;

			sql::Statement* stmt = con->createStatement();
			sql::ResultSet* res = stmt->executeQuery("SELECT LAST_INSERT_ID() AS OrderID");

			if (res->next())
				orderID = res->getInt("OrderID");

			delete res;
			delete stmt;

			cout << GREEN << "\nOrder Created!" << RESET << endl;
			cout << "Order ID : " << orderID << endl;
		}
		catch (sql::SQLException& e) {
			cout << RED << "\n[!] Error creating order: " << e.what() << RESET << endl;
			continue;
		}

		int catID, qty;
		double total = 0.0;
		OrderItem orderList[50];
		int itemCount = 0;

		bool addingItems = true;
		bool showMenuInput = true; 

		while (addingItems) {

			if (showMenuInput) {
				displayMenu(con);

				cout << "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
				cout << CYAN << "                         Morning Nest Cafe" << RESET << endl;
				cout << BLUE << "                               Orders" << RESET << endl;
				cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
				cout << "\nEnter ID: ";
				cin >> catID;
				cout << "Quantity: ";
				cin >> qty;

				if (qty <= 0) {
					cout << "\n\-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
					cout << CYAN << "            Morning Nest Cafe" << RESET << endl;
					cout << BLUE << "                 Orders" << RESET << endl;
					cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
					cout << RED << "\n[!] Invalid quantity." << RESET << endl;
					continue;
				}

				try {
					sql::PreparedStatement* pstmt = con->prepareStatement(
						"SELECT MenuName, PriceHot_RM, PriceCold_RM, Price_RM "
						"FROM menu WHERE CatID=? AND Availability='Yes'");

					pstmt->setInt(1, catID);
					sql::ResultSet* res = pstmt->executeQuery();

					if (!res->next()) {
						cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
						cout << CYAN << "            Morning Nest Cafe" << RESET << endl;
						cout << BLUE << "                 Orders" << RESET << endl;
						cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
						cout << RED << "\n[!] Menu not available or invalid ID." << RESET << endl;
						delete res;
						delete pstmt;
						continue;
					}

					string menuName = res->getString("MenuName");

					bool drinkHot = !res->isNull("PriceHot_RM");
					bool drinkCold = !res->isNull("PriceCold_RM");

					double price = 0.0;
					string drinkType = "-";

					if (drinkHot && drinkCold) {
						int tempChoice;
						do {
							cout << "\nDrink Type:\n";
							cout << CYAN <<  "\n1- Hot  (RM " << res->getDouble("PriceHot_RM") << ")\n";
							cout << "2- Cold (RM " << res->getDouble("PriceCold_RM") << ")\n" << RESET;
							cout << "\nSelect: ";
							cin >> tempChoice;

							if (tempChoice == 1) {
								price = res->getDouble("PriceHot_RM");
								drinkType = "Hot";
							}
							else if (tempChoice == 2) {
								price = res->getDouble("PriceCold_RM");
								drinkType = "Cold";
							}
							else {
								cout << RED << "\n[!] Invalid choice. Try again.\n" << RESET;
							}
						} while (price == 0.0);
					}
					else if (drinkHot) {
						price = res->getDouble("PriceHot_RM");
						drinkType = "Hot";
					}
					else if (drinkCold) {
						price = res->getDouble("PriceCold_RM");
						drinkType = "Cold";
					}
					else {
						price = res->getDouble("Price_RM");
					}

					double subtotal = price * qty;

					orderList[itemCount++] = { catID, menuName, drinkType, qty, price, subtotal };
					total += subtotal;

					delete res;
					delete pstmt;

					pstmt = con->prepareStatement(
						"INSERT INTO order_line (OrderID, CatID, Quantity, DrinkType, Subtotal_RM) "
						"VALUES (?, ?, ?, ?, ?)");
					pstmt->setInt(1, orderID);
					pstmt->setInt(2, catID);
					pstmt->setInt(3, qty);
					pstmt->setString(4, drinkType);
					pstmt->setDouble(5, subtotal);
					pstmt->executeUpdate();
					delete pstmt;

					cout << GREEN << "\nOrder added!" << RESET << endl;
				}
				catch (sql::SQLException& e) {
					cout << RED << "\n[!] Error adding item: " << e.what() << RESET << endl;
				}
			}

			showMenuInput = false;
			                     
			cout << YELLOW << "\n---------------------------------------------------------------------\n" << RESET;
			cout << "                         Current Order Items\n";
			cout << YELLOW << "---------------------------------------------------------------------\n" << RESET;
			cout << left << setw(5) << "No." << setw(25) << "Menu" << setw(10) << "Type"
				<< setw(8) << "Qty" << setw(12) << "Amount(RM)" << endl;
			cout << YELLOW << "---------------------------------------------------------------------\n" << RESET;

			for (int i = 0; i < itemCount; i++) {
				cout << left << setw(5) << (i + 1)
					<< setw(25) << orderList[i].menuName
					<< setw(10) << orderList[i].drinkType
					<< setw(8) << ("x" + to_string(orderList[i].qty))
					<< setw(12) << fixed << setprecision(2) << orderList[i].subtotal << endl;
			}
			cout << YELLOW << "---------------------------------------------------------------------\n" << RESET;
			cout << "Total: RM " << fixed << setprecision(2) << total << endl;
			cout << YELLOW << "---------------------------------------------------------------------\n" << RESET;

			cout << CYAN <<  "\n1- Add another item\n";
			cout << "2- Remove an item\n";
			cout << "3- Proceed to payment\n";
			cout << "0- Cancel entire order\n" << RESET;
			cout << "\nSelect option: ";

			int orderOption;
			cin >> orderOption;

			if (orderOption == 1) {
				showMenuInput = true; 
				continue;
			}
			else if (orderOption == 2) {
				if (itemCount == 0) {
					cout << RED << "\n[!] No items to remove!" << RESET << endl;
					continue;
				}

				int removeIndex;
				cout << "\nEnter item number to remove (1-" << itemCount << "): ";
				cin >> removeIndex;
				removeIndex--;

				if (removeIndex < 0 || removeIndex >= itemCount) {
					cout << RED << "\n[!] Invalid item number!" << RESET << endl;
					continue;
				}

				try {
					sql::PreparedStatement* pstmt = con->prepareStatement(
						"DELETE FROM order_line WHERE OrderID = ? AND CatID = ? AND DrinkType = ? LIMIT 1"
					);
					pstmt->setInt(1, orderID);
					pstmt->setInt(2, orderList[removeIndex].CatID);
					pstmt->setString(3, orderList[removeIndex].drinkType);
					pstmt->executeUpdate();
					delete pstmt;

					cout << GREEN << "\nItem '" << orderList[removeIndex].menuName
						<< "' removed successfully!" << RESET << endl;

					total -= orderList[removeIndex].subtotal;

					for (int i = removeIndex; i < itemCount - 1; i++) {
						orderList[i] = orderList[i + 1];
					}
					itemCount--;

				}
				catch (sql::SQLException& e) {
					cout << RED << "\n[!] Error removing item: " << e.what() << RESET << endl;
				}
			}
			else if (orderOption == 3) {

				if (itemCount == 0) {
					cout << RED << "\n[!] Cannot proceed with empty order!" << RESET << endl;
					continue;
				}
				addingItems = false;
			}
			else if (orderOption == 0) {

				try {
					sql::PreparedStatement* pstmt = con->prepareStatement(
						"DELETE FROM orders WHERE OrderID = ?"
					);
					pstmt->setInt(1, orderID);
					pstmt->executeUpdate();
					delete pstmt;

					cout << RED << "\n[!] Entire order cancelled." << RESET << endl;
					cout << "\nReturning to main menu...\n";
					orderComplete = true;
					addingItems = false;
				}
				catch (sql::SQLException& e) {
					cout << RED << "\n[!] Error cancelling order: " << e.what() << RESET << endl;
				}
				break;
			}
			else {
				cout << RED << "\n[!] Invalid option!" << RESET << endl;
			}
		}

		if (orderComplete) {
			break;
		}

		try {
			sql::PreparedStatement* pstmt = con->prepareStatement(
				"UPDATE orders SET TotalPrice_RM = ? WHERE OrderID = ?"
			);
			pstmt->setDouble(1, total);
			pstmt->setInt(2, orderID);
			pstmt->executeUpdate();
			delete pstmt;

			cout << YELLOW << "\n---------------------------------------------------------------------" << RESET << endl;
			cout << "                            Order Summary" << endl;
			cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;
			cout << "Order ID      : " << orderID << endl;
			cout << "Customer Name : " << custName << endl;
			cout << "Staff ID      : " << staffID << endl;
			cout << "Order Source  : " << orderSource << RESET << endl;
			cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;
			cout << left << setw(25) << "Menu" << setw(10) << "Type"
				<< setw(8) << "Qty" << setw(12) << "Amount(RM)" << endl;
			cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;

			for (int i = 0; i < itemCount; i++) {
				cout << left << setw(25) << orderList[i].menuName
					<< setw(10) << orderList[i].drinkType
					<< setw(8) << ("x" + to_string(orderList[i].qty))
					<< setw(12) << fixed << setprecision(2) << orderList[i].subtotal << endl;
			}

			cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;
			cout << "Total: RM " << fixed << setprecision(2) << total << endl;
			cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;
		}
		catch (sql::SQLException& e) {
			cout << RED << "\n[!]Error finalizing order: " << e.what() << RESET << endl;
		}

		char proceedPay;
		cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << CYAN << "                   Morning Nest Cafe" << RESET << endl;
		cout << BLUE << "                         Payment" << RESET << endl;
		cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << "\nProceed to payment? (" << GREEN << "Yes" << RESET << "/" << RED << "No" << RESET "): ";
		cin >> proceedPay;

		if (proceedPay == 'Y' || proceedPay == 'y') {
			payment(con, orderID);
			orderComplete = true;
		}
		else {
			try {
				sql::PreparedStatement* pstmt = con->prepareStatement(
					"DELETE FROM orders WHERE OrderID = ?"
				);
				pstmt->setInt(1, orderID);
				pstmt->executeUpdate();
				delete pstmt;

				cout << RED << "\n[!] Order cancelled." << RESET << endl;
				cout << "Payment is required to proceed. Starting new order...\n";
			}
			catch (sql::SQLException& e) {
				cout << RED << "\n[!] Error cancelling unpaid order: " << e.what() << RESET << endl;
			}

		}
	}
}

// Payment and Sales Generating Module

// (Staff) Payment Module: Membership - Member Confirmation
bool checkMembership(sql::Connection* con, int& memberID) {
	char choice;
	memberID = -1;

	cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
	cout << CYAN << "                   Morning Nest Cafe" << RESET << endl;
	cout << BLUE << "                      Membership" << RESET << endl;
	cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
	cout << "\nDoes customer have membership? (" << GREEN << "Yes" << RESET << "/" << RED << "No" << RESET "): ";
	cin >> choice;

	if (choice == 'Y' || choice == 'y') {
		cout << "Enter Member ID: ";
		cin >> memberID;

		try {
			sql::PreparedStatement* pstmt =
				con->prepareStatement(
					"SELECT MemberID FROM membership WHERE MemberID = ?"
				);
			pstmt->setInt(1, memberID);
			sql::ResultSet* res = pstmt->executeQuery();

			if (res->next()) {
				cout << GREEN << "\nMembership verified!" << RESET << endl;
				delete res;
				delete pstmt;
				return true;
			}

			cout << RED << "\n[!] Invalid Member ID." << RESET << endl;
			memberID = -1;
			delete res;
			delete pstmt;
			return false;
		}
		catch (sql::SQLException& e) {
			cout << RED << "\n[!] Error checking membership: " << e.what() << RESET << endl;
			memberID = -1;
			return false;
		}
	}
	else {
		cout << "\nRegister new membership? (" << GREEN << "Yes" << RESET << "/" << RED << "No" << RESET "): ";
		cin >> choice;

		if (choice == 'Y' || choice == 'y') {
			registerMembership(con, memberID);
			return (memberID != -1);
		}
	}

	return false;
}

// (Staff) Payment Module: Membership - Member Registering
void registerMembership(sql::Connection* con, int& memberID) {
	string memberName;
	int memPhoneNum;

	cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
	cout << BLUE << "                      Membership" << RESET << endl;
	cout << CYAN << "                Membership Registration" << RESET << endl;
	cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
	cout << "\nCustomer Name: ";
	getline(cin >> ws, memberName);
	cout << "Phone Number: ";
	cin >> memPhoneNum;

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"INSERT INTO membership "
				"(MemberName, MPhoneNum, PointMember, MemLevel, MemDisc) "
				"VALUES (?, ?, 0, 'Bronze', 0.05)"
			);

		pstmt->setString(1, memberName);
		pstmt->setInt(2, memPhoneNum);
		pstmt->executeUpdate();
		delete pstmt;

		sql::Statement* stmt = con->createStatement();
		sql::ResultSet* res =
			stmt->executeQuery("SELECT LAST_INSERT_ID() AS MemberID");

		if (res->next())
			memberID = res->getInt("MemberID");

		delete res;
		delete stmt;

		cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
		cout << BLUE << "                      Membership" << RESET << endl;
		cout << CYAN << "                Membership Registration" << RESET << endl;
		cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n";
		cout << GREEN << "\nMembership created successfully.\n" << RESET;
		cout << CYAN << "\nMember ID    : " << memberID << endl;
		cout << "Member Name  : " << memberName << endl;
		cout << "Phone Number : " << memPhoneNum << RESET << endl;
	}
	catch (sql::SQLException& e) {
		cout << "\n[!] Error registering membership: " << e.what() << endl;
		memberID = -1;
	}
}

double cashRounding(double amount) {
	int sen = (int)(amount * 100 + 0.0001);
	int lastDigit = sen % 10;

	if (lastDigit <= 2)
		sen -= lastDigit;
	else if (lastDigit <= 4)
		sen += (5 - lastDigit);
	else if (lastDigit <= 7)
		sen -= (lastDigit - 5);
	else
		sen += (10 - lastDigit);

	return sen / 100.0;
}

// (Staff) Payment Module : Payment
void payment(sql::Connection* con, int orderID) {

	double subtotal = 0.0, discountRate = 0.0;
	double discountAmt, afterDisc, tax, finalPrice;
	int memberID = -1;
	bool mncMember = false;

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement("SELECT TotalPrice_RM FROM orders WHERE OrderID=?");
		pstmt->setInt(1, orderID);
		sql::ResultSet* res = pstmt->executeQuery();

		if (res->next())
			subtotal = res->getDouble("TotalPrice_RM");

		delete res;
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "\n[!] Error retrieving order total." << e.what() << RESET << endl;
		return;
	}

	mncMember = checkMembership(con, memberID);

	if (mncMember && memberID != -1) {
		sql::PreparedStatement* pstmt =
			con->prepareStatement("SELECT MemDisc FROM membership WHERE MemberID=?");
		pstmt->setInt(1, memberID);
		sql::ResultSet* res = pstmt->executeQuery();

		if (res->next())
			discountRate = res->getDouble("MemDisc");

		delete res;
		delete pstmt;
	}

	discountAmt = subtotal * discountRate;
	afterDisc = subtotal - discountAmt;
	tax = afterDisc * 0.06;
	finalPrice = afterDisc + tax;

	int method = 0;
	string payMethod, fdType = "-";


	while (true) {
		cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << CYAN << "                      Morning Nest Cafe" << RESET << endl;
		cout << BLUE << "                           Payment" << RESET << endl;
		cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
		cout << "\nPayment Method:\n";
		cout << CYAN << "1- Cash\t\t2- Card\t\t3- e-Wallet\t4- Food Delivery\n" << RESET;
		cout << "Select: ";
		cin >> method;

		if (method == 1) {
			payMethod = "Cash";
			break;
		}
		else if (method == 2) {
			payMethod = "Card";
			break;
		}
		else if (method == 3) {
			payMethod = "e-Wallet";
			break;
		}
		else if (method == 4) {
			payMethod = "FoodDelivery";

			int fd;
			while (true) {
				cout << "\nFood Delivery Type:\n";
				cout << CYAN << "\n1- FP\t2- SF\t3- GF" << RESET << endl;
				cout << "Select: ";
				cin >> fd;

				if (fd == 1) { fdType = "FP"; break; }
				else if (fd == 2) { fdType = "SF"; break; }
				else if (fd == 3) { fdType = "GF"; break; }
				else cout << "\nInvalid FD type. Try again.\n";
			}
			break;
		}
		else {
			cout << RED << "\n[!] Invalid payment method. Please try again." << RESET << endl;
		}
	} 

	if (payMethod == "Cash") {
		finalPrice = cashRounding(finalPrice);
	}


	double paid = finalPrice, change = 0.0;
	if (payMethod == "Cash") {
		while (true) {
			cout << "Amount Paid (RM): ";
			cin >> paid;

			if (paid >= finalPrice) {
				change = paid - finalPrice;
				break;
			}
			cout << RED << "[!] Insufficient cash. Try again." << RESET << endl;
		}
		change = paid - finalPrice;
	}

	if (mncMember && memberID != -1) {
		int memberPoints = (int)(finalPrice * 10);

		sql::PreparedStatement*
			pstmt = con->prepareStatement("UPDATE membership SET PointMember = PointMember + ? WHERE MemberID=?");
		pstmt->setInt(1, memberPoints);
		pstmt->setInt(2, memberID);
		pstmt->executeUpdate();
		delete pstmt;
	}

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"INSERT INTO payment ("
				"OrderID, Subtotal_RM, PayMethod, FDType, "
				"TotalPrice_RM, PriceDisc_RM, Tax_RM, MemberID, "
				"FinalPrice_RM, Paid_RM, Change_RM"
				") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
			);

		pstmt->setInt(1, orderID);
		pstmt->setDouble(2, subtotal);
		pstmt->setString(3, payMethod);
		pstmt->setString(4, fdType);
		pstmt->setDouble(5, afterDisc);
		pstmt->setDouble(6, discountAmt);
		pstmt->setDouble(7, tax);

		if (mncMember)
			pstmt->setInt(8, memberID);
		else
			pstmt->setNull(8, sql::DataType::INTEGER);

		pstmt->setDouble(9, finalPrice);
		pstmt->setDouble(10, paid);
		pstmt->setDouble(11, change);

		pstmt->executeUpdate();
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "\n[!] Error saving payment: " << e.what() << RESET << endl;
		return;
	}

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"UPDATE orders SET OrderStatus='Completed', OrderHistory='Paid' WHERE OrderID=?"
			);
		pstmt->setInt(1, orderID);
		pstmt->executeUpdate();
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "\n[!] Error in updating order status." << e.what() << RESET << endl;
	}
	                   
	cout << YELLOW << "\n---------------------------------------------------------------------" << RESET << endl;
	cout << "                            Morning Nest Cafe" << endl;
	cout << "             Address: Universiti Teknikal Malaysia Melaka," << endl;
	cout << "                Kafeteria Eksekutif, Jalan Hang Tuah Jaya," << endl;
	cout << "                      76100 Durian Tunggal, Melaka" << endl;
	cout << "                             Tel: 123-456-789" << endl;
	cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;
	
	string staffName = "-";
	try { 
		sql::PreparedStatement* pstmtStaff =
			con->prepareStatement(
				"SELECT s.StaffName "
				"FROM orders o "
				"JOIN staff s ON o.StaffID = s.StaffID "
				"WHERE o.OrderID = ?"
			);
		pstmtStaff->setInt(1, orderID);
		sql::ResultSet* rsStaff = pstmtStaff->executeQuery();

		if (rsStaff->next())
			staffName = rsStaff->getString("StaffName");

		delete rsStaff;
		delete pstmtStaff;
	}

	catch (sql::SQLException& e) {
		string staffName = "-";
	}

	string custName = "Walk-in";

	try {
		sql::PreparedStatement* pstmtCust =
			con->prepareStatement(
				"SELECT CustomerName FROM orders WHERE OrderID = ?"
			);
		pstmtCust->setInt(1, orderID);
		sql::ResultSet* rsCust = pstmtCust->executeQuery();

		if (rsCust->next() && !rsCust->isNull("CustomerName"))
			custName = rsCust->getString("CustomerName");

		delete rsCust;
		delete pstmtCust;
	}
	catch (sql::SQLException& e) {
		custName = "Walk-in";
	}

	cout << "Order ID          : " << orderID << endl;
	cout << "Cashier           : " << staffName << endl;
	cout << "Customer Name     : " << custName << endl;
	cout << "Payment Method    : " << payMethod << endl;
	if (payMethod == "FoodDelivery")
		cout << "Food Delivery     : " << fdType << endl;
	
	try {
		sql::PreparedStatement* pstmtItem =
			con->prepareStatement(
				"SELECT m.MenuName, ol.DrinkType, ol.Quantity, "
				"CASE "
				" WHEN ol.DrinkType='Hot' THEN m.PriceHot_RM "
				" WHEN ol.DrinkType='Cold' THEN m.PriceCold_RM "
				" ELSE m.Price_RM END AS price, "
				"ol.Subtotal_RM "
				"FROM order_line ol "
				"JOIN menu m ON ol.CatID = m.CatID "
				"WHERE ol.OrderID = ?"
			);

		pstmtItem->setInt(1, orderID);
		sql::ResultSet* resItem = pstmtItem->executeQuery();

		cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;
		cout << left << setw(35) << "Menu" << setw(10) << "Type" << setw(6) << "Qty" << setw(10) << "Price" << setw(10) << "Amount" << RESET << endl;
		cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;

		while (resItem->next()) {
			cout << left
				<< setw(35) << resItem->getString("MenuName")
				<< setw(10) << (resItem->isNull("DrinkType") ? "-" : resItem->getString("DrinkType"))
				<< setw(6) << resItem->getInt("Quantity")
				<< setw(10) << fixed << setprecision(2) << resItem->getDouble("price")
				<< setw(10) << resItem->getDouble("Subtotal_RM")
				<< endl;
		}
		cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;
		cout << fixed << setprecision(2);
		cout << "Subtotal   : RM " << subtotal << endl;
		cout << "Discount   : RM " << discountAmt << endl;
		cout << "Tax (6%)   : RM " << tax << endl;
		cout << "TOTAL      : RM " << finalPrice << endl;

		if (payMethod == "Cash") {
			cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;
			cout << "Paid       : RM " << paid << endl;
			cout << "Change     : RM " << change << endl;
		}

		string memLevel = "-";
		int memPoint = 0;

		if (mncMember && memberID != -1) {
			try {
				sql::PreparedStatement* pstmt =
					con->prepareStatement(
						"SELECT MemLevel, PointMember "
						"FROM membership WHERE MemberID = ?"
					);
				pstmt->setInt(1, memberID);
				sql::ResultSet* rs = pstmt->executeQuery();

				if (rs->next()) {
					memLevel = rs->getString("MemLevel");
					memPoint = rs->getInt("PointMember");
				}

				delete rs;
				delete pstmt;
			}
			catch (sql::SQLException& e) {
				memLevel = "-";
				memPoint = 0;
			}
		}

		if (mncMember && memberID != -1) {
			cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;
			cout << "Member ID      : " << memberID << endl;
			cout << "Member Point   : " << memPoint << endl;
			cout << "Member Level   : " << memLevel << endl;
		}

		cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;
		cout << "                            Thank You!\n";
		cout << YELLOW << "---------------------------------------------------------------------" << RESET << endl;

	}
	catch (sql::SQLException& e) {
		cout << RED << "\n[!] Error Printing Receipt." << e.what() << RESET << endl;
	}
}

// (Admin) Sales Reporting: Sales Report
void salesReport(sql::Connection* con, string reportDate, int adminID, int& outsalesID) {

	double walkInSales = 0.0, gfSales = 0.0, fpSales = 0.0, sfSales = 0.0;
	double totalSales = 0.0, totalDisc = 0.0;
	int totalTxn = 0;

	string adminName = "-";

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement("SELECT AdminName FROM admin WHERE AdminID=?");
		pstmt->setInt(1, adminID);
		sql::ResultSet* rs = pstmt->executeQuery();

		if (rs->next())
			adminName = rs->getString("AdminName");

		delete rs;
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		adminName = "-";
	}

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"SELECT PayMethod, FDType, "
				"SUM(FinalPrice_RM) AS TotalSales, "
				"SUM(PriceDisc_RM) AS TotalDisc, "
				"COUNT(*) AS Txn "
				"FROM payment p "
				"JOIN orders o ON p.OrderID=o.OrderID "
				"WHERE DATE(p.PayTime)=? "
				"GROUP BY PayMethod, FDType"
			);

		pstmt->setString(1, reportDate);
		sql::ResultSet* rs = pstmt->executeQuery();

		bool hasData = false;

		while (rs->next()) {
			hasData = true;
			string method = rs->getString("PayMethod");
			string fd = rs->isNull("FDType") ? "-" : rs->getString("FDType");
			double sales = rs->getDouble("TotalSales");
			double disc = rs->getDouble("TotalDisc");
			int txn = rs->getInt("Txn");

			totalSales += sales;
			totalDisc += disc;
			totalTxn += txn;

			if (method == "Cash" || method == "Card" || method == "e-Wallet")
				walkInSales += sales;
			else if (fd == "GF")
				gfSales += sales;
			else if (fd == "FP")
				fpSales += sales;
			else if (fd == "SF")
				sfSales += sales;
		}

		delete rs;
		delete pstmt;

		if (!hasData) {
			cout << RED << "\n[!] No sales data found for date: "
				<< reportDate << RESET << endl;
			outsalesID = -1;
			return;
		}
	}
	catch (sql::SQLException& e) {
		cout << RED << "[Error] Sales calculation failed: " << e.what() << RESET << endl;
		outsalesID = -1;
		return;
	}

	double atv = (totalTxn > 0) ? (totalSales / totalTxn) : 0.0;

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"INSERT INTO SALES "
				"(WalkInSales, GFSales, FPSales, SFSales, "
				"TotalOrder, TotalDisc_RM) "
				"VALUES (?, ?, ?, ?, ?, ?)"
			);

		pstmt->setDouble(1, walkInSales);
		pstmt->setDouble(2, gfSales);
		pstmt->setDouble(3, fpSales);
		pstmt->setDouble(4, sfSales);
		pstmt->setDouble(5, totalSales);
		pstmt->setDouble(6, totalDisc);

		pstmt->executeUpdate();
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "[!] Error inserting SALES failed: " << e.what() << RESET << endl;
		outsalesID = -1;
		return;
	}

	try {
		sql::Statement* stmt = con->createStatement();
		sql::ResultSet* rs = stmt->executeQuery("SELECT LAST_INSERT_ID() AS SalesID");

		if (rs->next())
			outsalesID = rs->getInt("SalesID");

		delete rs;
		delete stmt;
	}
	catch (sql::SQLException& e) {
		outsalesID = -1;
	}

	cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	cout << BLUE << "                  Sales Reporting Management" << RESET << endl;
	cout << CYAN << "                         Sales Report" << RESET << endl;
	cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;

	cout << "\n=====================================================================" << endl;
	cout << CYAN << "                            Morning Nest Cafe" << endl;
	cout << "             Address: Universiti Teknikal Malaysia Melaka," << endl;
	cout << "                Kafeteria Eksekutif, Jalan Hang Tuah Jaya," << endl;
	cout << "                      76100 Durian Tunggal, Melaka" << endl;
	cout << "                             Tel: 123-456-789" << RESET << endl;
	cout << "---------------------------------------------------------------------" << endl;
	cout << "---------------------------------------------------------------------" << endl;
	cout << CYAN << "                    Sales Performance Report" << RESET << endl;
	cout << CYAN << "                         Date : " << reportDate << RESET << endl;
	cout << CYAN << "                  Generated by: " << adminName << RESET << endl;
	cout << "---------------------------------------------------------------------" << endl;

	cout << fixed << setprecision(2);
	cout << CYAN << "[1] Sales by Channel" << RESET << endl;
	cout << "---------------------------------------------------------------------" << endl;
	cout << "Walk-in Sales :" << GREEN << "RM " << walkInSales << RESET << endl;
	cout << "GrabFood      :" << GREEN << "RM " << gfSales << RESET << endl;
	cout << "FoodPanda     :" << GREEN << "RM " << fpSales << RESET << endl;
	cout << "ShopeeFood    :" << GREEN << "RM " << sfSales << RESET << endl;
	cout << "\nTotal Orders  :" << GREEN << "RM " << totalTxn << RESET << endl;
	cout << "Total Sales   :" << GREEN << "RM " << totalSales << RESET << endl;
	cout << "ATV           :" << GREEN << "RM " << atv << RESET << endl;
	cout << "---------------------------------------------------------------------" << endl;
	cout << CYAN << "[2] Best Selling Menu (by Quantity)" << RESET << endl;
	cout << "---------------------------------------------------------------------" << endl;
	
	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"SELECT m.MenuName, SUM(ol.Quantity) AS Sold "
				"FROM order_line ol "
				"JOIN menu m ON ol.CatID = m.CatID "
				"JOIN orders o ON ol.OrderID = o.OrderID "
				"JOIN payment p ON p.OrderID = o.OrderID "
				"WHERE DATE(p.PayTime) = ? "
				"GROUP BY m.MenuName "
				"ORDER BY Sold DESC"
			);

		pstmt->setString(1, reportDate);
		sql::ResultSet* rs = pstmt->executeQuery();

		bool hasMenu = false;
		while (rs->next()) {
			hasMenu = true;
			cout << left
				<< setw(35) << rs->getString("MenuName")
				<< "x" << rs->getInt("Sold") << endl;
		}

		if (!hasMenu) {
			cout << YELLOW << "[!] No menu items found for this date." << RESET << endl;
		}

		delete rs;
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "[!] Unable to retrieve best selling menu." << RESET << endl;
		cout << "Error: " << e.what() << endl; 
	}

	cout << "---------------------------------------------------------------------" << endl;
	cout << CYAN << "                      End Of Sales Report" << RESET << endl;
	cout << "---------------------------------------------------------------------" << endl;
	cout << "\n=====================================================================" << endl;
}

// (Admin) Sales Reporting Module: Order Trend Report
void orderTrend(sql::Connection* con, string date, int salesID, int adminID) {

	struct HourSlot {
		string label;
		string start;
		string end;
		string period;
	};

	HourSlot hours[] = {
		{"10-11", "10:00:00", "10:59:59", "Morning"},
		{"11-12", "11:00:00", "11:59:59", "Morning"},
		{"12-13", "12:00:00", "12:59:59", "Afternoon"},
		{"13-14", "13:00:00", "13:59:59", "Afternoon"},
		{"14-15", "14:00:00", "14:59:59", "Afternoon"},
		{"15-16", "15:00:00", "15:59:59", "Afternoon"},
		{"16-17", "16:00:00", "16:59:59", "Afternoon"},
		{"17-18", "17:00:00", "17:59:59", "Evening"},
		{"18-19", "18:00:00", "18:59:59", "Evening"},
		{"19-20", "19:00:00", "19:59:59", "Evening"},
		{"20-21", "20:00:00", "20:59:59", "Evening"},
		{"21-22", "21:00:00", "21:59:59", "Evening"}
	};

	struct Period {
		string name;
		string start;
		string end;
		double hours;
	};

	Period periods[] = {
		{"Morning",   "10:00:00", "11:59:59", 2.0},
		{"Afternoon", "12:00:00", "16:59:59", 5.0},
		{"Evening",   "17:00:00", "21:59:59", 5.0}
	};

	const double serviceRate = 12.0;

	string adminName = "-";
	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement("SELECT AdminName FROM admin WHERE AdminID=?");
		pstmt->setInt(1, adminID);
		sql::ResultSet* rs = pstmt->executeQuery();

		if (rs->next())
			adminName = rs->getString("AdminName");

		delete rs;
		delete pstmt;
	}

	catch (sql::SQLException& e) {
		adminName = "-";
	}

	cout << "\n\n *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	cout << BLUE << "                           Sales Reporting Management" << RESET << endl;
	cout << CYAN << "                                Order Trend Report" << RESET << endl;
	cout << " *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;

	cout << "\n===========================================================================" << endl;
	cout << CYAN << "                             Morning Nest Cafe" << endl;
	cout << "             Address: Universiti Teknikal Malaysia Melaka," << endl;
	cout << "                Kafeteria Eksekutif, Jalan Hang Tuah Jaya," << endl;
	cout << "                      76100 Durian Tunggal, Melaka" << endl;
	cout << "                             Tel: 123-456-789" << RESET << endl;
	cout << "---------------------------------------------------------------------------" << endl;
	cout << "---------------------------------------------------------------------------" << endl;
	cout << CYAN << "                           Order Trend Report" << RESET << endl;
	cout << CYAN << "                             Date: " << date << RESET << endl;
	cout << CYAN << "                         Generated by: " << adminName << RESET << endl;
	cout << "---------------------------------------------------------------------------" << endl;

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"DELETE FROM OTREND WHERE TimeDate=? AND SalesID=?"
			);
		pstmt->setString(1, date);
		pstmt->setInt(2, salesID);
		pstmt->executeUpdate();
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "[!] Error to clear old OTREND data" << RESET << endl;
	}

	cout << CYAN << "\n[1] Hourly Order Analysis" << RESET << endl;
	cout << "---------------------------------------------------------------------------" << endl;

	cout << left
		<< setw(10) << "Hour"
		<< setw(12) << "Period"
		<< setw(10) << "Orders"
		<< setw(15) << "Utilization"
		<< setw(10) << "Peak"
		<< endl;
	cout << "---------------------------------------------------------------------------" << endl;

	for (auto& h : hours) {

		int hourlyCount = 0;

		try {
			sql::PreparedStatement* pstmt =
				con->prepareStatement(
					"SELECT COUNT(*) AS count "
					"FROM payment "
					"WHERE DATE(PayTime)=? AND TIME(PayTime) BETWEEN ? AND ?"
				);

			pstmt->setString(1, date);
			pstmt->setString(2, h.start);
			pstmt->setString(3, h.end);

			sql::ResultSet* rs = pstmt->executeQuery();
			if (rs->next())
				hourlyCount = rs->getInt("count");

			delete rs;
			delete pstmt;
		}
		catch (sql::SQLException& e) {
			hourlyCount = 0;
		}

		// Arrival rate = orders per hour (for 1 hour period)
		double arrivalRate = (double)hourlyCount / 1.0;  // λ = orders/hour
		double utilization = arrivalRate / serviceRate;   // ρ = λ/μ

		string peak;
		if (utilization >= 1.0)
			peak = "Overload";
		else if (utilization >= 0.8)
			peak = "High";
		else if (utilization >= 0.5)
			peak = "Medium";
		else
			peak = "Low";


		string peakColor;
		if (peak == "Overload")
			peakColor = RED;
		else if (peak == "High")
			peakColor = GREEN;
		else if (peak == "Medium")
			peakColor = YELLOW;
		else
			peakColor = RED;

		cout << left
			<< setw(10) << h.label
			<< setw(12) << h.period
			<< setw(10) << hourlyCount
			<< setw(15) << fixed << setprecision(2) << utilization
			<< peakColor << setw(10) << peak << RESET
			<< endl;
	}

	cout << "---------------------------------------------------------------------------" << endl;
	cout << CYAN << "\n[2] Period Summary" << RESET << endl;
	cout << "---------------------------------------------------------------------------" << endl;

	cout << left
		<< setw(12) << "Period"
		<< setw(10) << "Orders"
		<< setw(15) << "Arrival Rate"
		<< setw(15) << "Service Rate"
		<< setw(15) << "Utilization"
		<< setw(10) << "Peak"
		<< endl;
	cout << "---------------------------------------------------------------------------" << endl;

	for (int i = 0; i < 3; i++) {

		int orderCount = 0;

		try {
			sql::PreparedStatement* pstmt =
				con->prepareStatement(
					"SELECT COUNT(*) AS count "
					"FROM payment p "
					"WHERE DATE(p.PayTime)=? "
					"AND TIME(p.PayTime) BETWEEN ? AND ?"
				);

			pstmt->setString(1, date);
			pstmt->setString(2, periods[i].start);
			pstmt->setString(3, periods[i].end);

			sql::ResultSet* rs = pstmt->executeQuery();
			if (rs->next())
				orderCount = rs->getInt("count");

			delete rs;
			delete pstmt;
		}
		catch (sql::SQLException& e) {
			orderCount = 0;
		}

		// λ = arrival rate (orders per hour)
		double arrivalRate = (double)orderCount / periods[i].hours;
		// ρ = utilization (λ/μ)
		double utilization = (arrivalRate > 0) ? (arrivalRate / serviceRate) : 0.0;

		double avgQLength = 0.0;
		double avgWaitTime = 0.0;

		// Calculate avgQLength first, then avgWaitTime
		if (utilization > 0 && utilization < 1) {
			avgQLength = (utilization * utilization) / (1 - utilization);
			avgWaitTime = avgQLength / arrivalRate;
		}

		// Use order count based peak status
		string peak;
		if (orderCount >= 25)
			peak = "High";
		else if (orderCount >= 10)
			peak = "Medium";
		else if (orderCount > 0)
			peak = "Low";
		else
			peak = "No Orders";

		string peakColor;
		if (peak == "High")
			peakColor = GREEN;
		else if (peak == "Medium")
			peakColor = YELLOW;
		else if (peak == "Low")
			peakColor = RED;
		else
			peakColor = RED;

		try {
			sql::PreparedStatement* pstmt =
				con->prepareStatement(
					"INSERT INTO OTREND "
					"(TimeDate, TimePeriod, OrderCount, ArrivalRate, ServiceRate, "
					"Utilization, AvgQLength, AvgWaitTime, PeakStatus, SalesID) "
					"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
				);

			pstmt->setString(1, date);
			pstmt->setString(2, periods[i].name);
			pstmt->setInt(3, orderCount);
			pstmt->setDouble(4, arrivalRate);
			pstmt->setDouble(5, serviceRate);
			pstmt->setDouble(6, utilization);
			pstmt->setDouble(7, avgQLength);
			pstmt->setDouble(8, avgWaitTime);
			pstmt->setString(9, peak);
			pstmt->setInt(10, salesID);

			pstmt->executeUpdate();
			delete pstmt;
		}
		catch (sql::SQLException& e) {
			cout << RED << "[Warning] Failed insert OTREND for " << periods[i].name << RESET << endl;
		}

		cout << left
			<< setw(12) << periods[i].name
			<< setw(10) << orderCount
			<< setw(15) << fixed << setprecision(2) << arrivalRate
			<< setw(15) << serviceRate
			<< setw(15) << utilization
			<< peakColor << setw(10) << peak << RESET
			<< endl;

		cout << " [3] Avg Queue Length : " << YELLOW << fixed << setprecision(2) << avgQLength << RESET << endl;
		cout << " [4] Avg Waiting Time : " << YELLOW << avgWaitTime << " hours" << RESET << endl;
		cout << "---------------------------------------------------------------------------" << endl;
	}

	cout << CYAN << "                     End Of Order Trend Report" << RESET << endl;
	cout << "---------------------------------------------------------------------------" << endl;
	cout << "===========================================================================" << endl;
}

// (Admin) Report Generating Module: Final Report
void summaryReport(sql::Connection* con, string reportDate, int adminID, int salesID) {

	int trendID = -1;
	string adminName = "-";
	string reportStart = reportDate + " 10:00:00";
	string reportEnd = reportDate + " 22:00:00";

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"SELECT AdminName FROM admin WHERE AdminID=?"
			);
		pstmt->setInt(1, adminID);
		sql::ResultSet* rs = pstmt->executeQuery();

		if (rs->next())
			adminName = rs->getString("AdminName");

		delete rs;
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		adminName = "-";
	}

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"SELECT TrendID FROM OTREND "
				"WHERE TimeDate=? AND SalesID=? "
				"ORDER BY TrendID DESC LIMIT 1"
			);
		pstmt->setString(1, reportDate);
		pstmt->setInt(2, salesID);
		sql::ResultSet* rs = pstmt->executeQuery();

		if (rs->next())
			trendID = rs->getInt("TrendID");

		delete rs;
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		trendID = -1;
	}

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"INSERT INTO REPORT "
				"(ReportDate, ReportStart, ReportEnd, SalesID, AdminID) "
				"VALUES (?, ?, ?, ?, ?)"
			);

		pstmt->setString(1, reportDate);
		pstmt->setString(2, reportStart);
		pstmt->setString(3, reportEnd);
		pstmt->setInt(4, salesID);
		pstmt->setInt(5, adminID);

		pstmt->executeUpdate();
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "\n[Error] Insert REPORT failed" << RESET << endl;
		cout << e.what() << endl;
		return;
	}

	cout << "\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	cout << BLUE << "                 Sales Reporting Management" << RESET << endl;
	cout << CYAN << "                      Summary Report" << RESET << endl;
	cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	                   
	cout << "\n=============================================================" << endl;
	cout << CYAN << "                            Morning Nest Cafe" << endl;
	cout << "             Address: Universiti Teknikal Malaysia Melaka," << endl;
	cout << "                Kafeteria Eksekutif, Jalan Hang Tuah Jaya," << endl;
	cout << "                      76100 Durian Tunggal, Melaka" << endl;
	cout << "                             Tel: 123-456-789" << RESET << endl;
	cout << "-------------------------------------------------------------" << endl;
	cout << "-------------------------------------------------------------" << endl;
	cout << CYAN"                       Summary Report" << RESET << endl;
	cout << "-------------------------------------------------------------" << endl;
	cout << CYAN << "Report Date   : " << reportDate << RESET << endl;
	cout << CYAN << "Report Period : " << reportStart << " to " << reportEnd << RESET << endl;
	cout << CYAN << "Generated By  : " << adminName << RESET << endl;
	cout << "-------------------------------------------------------------" << endl;
	cout << "Sales ID      : " << salesID << endl;
	cout << "Trend ID      : " << (trendID == -1 ? "N/A" : to_string(trendID)) << endl;
	cout << "-------------------------------------------------------------" << endl;

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"SELECT WalkInSales, GFSales, FPSales, SFSales, "
				"TotalOrder, TotalDisc_RM "
				"FROM SALES WHERE SalesID=?"
			);
		pstmt->setInt(1, salesID);
		sql::ResultSet* rs = pstmt->executeQuery();

		if (rs->next()) {
			cout << CYAN << "\n" << "[1] Sales Summary" << RESET << endl;
			cout << "-------------------------------------------------------------"<< endl;
			cout << fixed << setprecision(2);
			cout << "Walk-In Sales :" << GREEN << "RM " << rs->getDouble("WalkInSales") << RESET << endl;
			cout << "GrabFood      :" << GREEN << "RM " << rs->getDouble("GFSales") << RESET << endl;
			cout << "FoodPanda     :" << GREEN << "RM " << rs->getDouble("FPSales") << RESET << endl;
			cout << "ShopeeFood    :" << GREEN << "RM " << rs->getDouble("SFSales") << RESET << endl;
			cout << "Total Orders  :" << GREEN << "RM " << rs->getDouble("TotalOrder") << RESET << endl;
			cout << "Total Discount:" << GREEN << "RM " << rs->getDouble("TotalDisc_RM") << RESET << endl;
		}

		delete rs;
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "[Warning] Unable to load sales summary" << RESET << endl;
	}

	cout << CYAN << "\n" << "[2] Order Trend Overview" << RESET << endl;
	cout << "-------------------------------------------------------------" << endl;
	cout << left
		<< setw(15) << "Period"
		<< setw(12) << "Orders"
		<< setw(15) << "Utilization"
		<< setw(12) << "Peak"
		<< endl;
	cout << "-------------------------------------------------------------" << endl;

	try {
		sql::PreparedStatement* pstmt =
			con->prepareStatement(
				"SELECT TimePeriod, OrderCount, Utilization, PeakStatus "
				"FROM OTREND "
				"WHERE TimeDate=? AND SalesID=? "
				"ORDER BY FIELD(TimePeriod, 'Morning', 'Afternoon', 'Evening')"
			);
		pstmt->setString(1, reportDate);
		pstmt->setInt(2, salesID);

		sql::ResultSet* rs = pstmt->executeQuery();

		while (rs->next()) {
			string peak = rs->getString("PeakStatus");

			string peakColor;
			if (peak == "Overload")
				peakColor = RED;
			else if (peak == "High")
				peakColor = GREEN;
			else if (peak == "Medium")
				peakColor = YELLOW;
			else
				peakColor = RED;

			cout << left
				<< setw(15) << rs->getString("TimePeriod")
				<< setw(12) << rs->getInt("OrderCount")
				<< setw(15) << fixed << setprecision(2) << rs->getDouble("Utilization")
				<< peakColor << setw(12) << peak << RESET
				<< endl;
		}

		delete rs;
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		cout << RED << "[Warning] Order trend data unavailable" << RESET << endl;
		cout << "Error: " << e.what() << endl;
	}

	cout << "-------------------------------------------------------------" << endl;
	cout << CYAN << "                  End Of Summary Report" << RESET << endl;
	cout << "-------------------------------------------------------------" << endl;
	cout << "=============================================================" << endl;
}