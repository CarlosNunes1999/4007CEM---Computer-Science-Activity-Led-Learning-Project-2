#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include "sqlite3.h"
#include <sstream>
using namespace std;
void Menu();

sqlite3* db;
char *zErrMsg = 0;
sqlite3_stmt *stmt;
int rc;
using SQLData = vector<string>;
using SQL_Data = vector<SQLData>;

vector<string> simple_tokenizer(string s,string del = ",")
{ 
    vector<string> ModuleIDS;
    int start = 0;
    int end = s.find(del);
    while (end != -1) {
         ModuleIDS.push_back(s.substr(start, end - start));
        start = end + del.size();
        end = s.find(del, start);
    }
    
   
   
    return ModuleIDS;
}

int callback(void *NotUsed, int argc, char **argv, char **azColName){
    for(int i = 0; i < argc; i++) {
        cout << azColName[i] << ": " << argv[i] << endl;
    }
    return 0;
}
static int select_callback(void *f_data, int num_field, char **value_field, char **field_col_name) {
   SQL_Data* DATA = static_cast<SQL_Data*>(f_data);
   try
   {
       DATA->emplace_back(value_field, value_field + num_field);
   }
   catch(...)
   {
       return 1;
   }
   return 0;
}
SQL_Data select_stmt(const char* stmt)
{
  SQL_Data data;  
  char *errmsg;
  int result = sqlite3_exec(db, stmt, select_callback, &data, &errmsg);
  if (result != SQLITE_OK) {
    cerr << "Error in select statement " << stmt << "[" << errmsg << "]\n";
  }


  return data;
}
void sqlstmt(const char* stmt)
{
  char *errmsg;
  int result = sqlite3_exec(db, stmt, 0, 0, &errmsg);
  if (result != SQLITE_OK) {
    cerr << "Error in select statement " << stmt << "[" << errmsg << "]\n";
  }
}




int main() {
   
    int option;

    int result;
    string user = "";
    string pass = "";
    string name = "";

    rc = sqlite3_open("database/timetables.db",&db);
    if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }
    cout << "1. Login \n2. Create Account \n3. Leave \n";
    cin >> option;
    cout << to_string(option);
    switch (option)
    {
    case 1:
    {
        cout    <<  " You are now on Login Menu \n";
        cout << "Whats your username? \n";
        cin >> user;
        cout << "Whats your password? \n";
        cin >> pass;
        string sql = "SELECT * FROM Users Where User='"+user+"' AND Password=='"+pass+"' " ;
        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
            printf("error: %s\n", sqlite3_errmsg(db));
            // or throw an exception
            sqlite3_finalize(stmt);
            return -1;
        }
        if (rc == SQLITE_DONE) // no result
            result = -1;
        else if (sqlite3_column_type(stmt, 0) == SQLITE_NULL) // result is NULL
            result = -1;
        else { // some valid result
            result = sqlite3_column_int(stmt, 0);
           
        }

        sqlite3_finalize(stmt);

        
        if (result < 0)
        {
            fprintf(stdout,"Account doesn't exist!\n");
            main();
        }
        else
            fprintf(stdout,"\x1B[32m Login Successfull\033[0m \n");
            Menu();
        break;
    }
    case 2:
    {
        cout    <<  " You are now on Create Account \n";
        cout << "Username? \n";
        cin >> user;
        cout << "Password? \n";
        cin >> pass;
        cout << "Staff Name? \n";
        cin >> name;
        
        string sql = "INSERT INTO Users (User,Password,Name) "  \
        "VALUES ('"+ user +"','"+pass+"','"+name+"'); ";
        
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
        
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            fprintf(stdout, "Account created Sucessfully\n");
        }
        sqlite3_close(db);
        main();
        break;
    }
    case 3: 
        exit(0);
        break;
    default:
        cout << "\x1B[31m Please select a option in the menu! \033[0m \n";
        break;
    }
    sqlite3_close(db);
    return 0;
}

void Menu()
{
    
    SQL_Data data = select_stmt("SELECT * FROM Modules");
    rc = sqlite3_open("database/timetables.db",&db);
    int option;
    cout << "Select one of the following:";
    cout << " \n 1. Add Student \n 2. Add Module \n 3. Add Classes \n 4. Add Rooms \n 5. Add Lecture \n 6. Show Student Information \n 7. Exit \n";
    cin >> option;
    
    
   switch (option)
   {
    case 1:
    {
        string name;
        ostringstream oss;
        int choise;
        vector<int> Modules;
        cout << "Name of the Student:";
        cin >> name;
        std::getline(cin,name);
        int counter = 0;
        int temp;
        if (data.empty())
        {
            cout << " \x1B[31m You need to add the modules first! \033[0m \n";
            break;
        }
        do
        {
            counter = 0;
            cout << "Choose the Modules for the Student \n";
            
            for (auto& SQLData : data) 
            {
                counter++;
                if(counter == data.size()) temp = stoi(SQLData[0]);
                cout << SQLData[0] << ". " << SQLData[1] << "\n";
            }
            cout << to_string(temp+1) << ". Exit " << "\n";
            
            cin >> choise;     
            if (choise != temp)Modules.push_back(choise);
        } while (choise != temp+1);
        for(auto item : Modules)
        {
            oss << item << ",";
            
        }
        string sql = "INSERT INTO Students (Name,Module_ID) "  \
        "VALUES ('"+name+"','"+oss.str()+"'); ";
    
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
     
        break;
    }
    case 2:
    {
        string mod_name;
        cout << "You're now adding a Module to the database \n\n";
        cout << "Module Name:\n";
        cin >> mod_name;
        std::getline(cin,mod_name);
        string sql = "INSERT INTO Modules (Name) VALUES ('"+mod_name+"'); ";
    
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    
        break;
    }
    case 3:
    {
        int counter = 0;
        string class_name;
        string time;
        int choise;
        int mod_choise;
        SQL_Data data_room = select_stmt("SELECT * FROM Rooms");
        
        if (data_room.empty())
        {
            cout << " \x1B[31m You need to add the Rooms first! \033[0m \n";
            break;
        }
        cout << "\x1B[32m Welcome to the Class Menu\033[0m  \n\n";
        cout << "Write the name of the class\n\n";
        cin >> class_name;
        std::getline(cin,class_name);
        cout << "Write the time of the class (Format 00:00PM)\n\n";
        cin >> time;
        std::getline(cin,time);
        cout << "Select the Room \n\n";

        cout << "Choose the Room for the Class \n";
        for (auto& SQLData : data_room) 
        {
            counter++;
            cout << SQLData[0] << ". " << SQLData[1] << "\n";
        }
        cin >> choise;     

        cout << "Choose the Module for the Class \n";
        for (auto& SQLData : data) 
        {
            counter++;
            cout << SQLData[0] << ". " << SQLData[1] << "\n";
        }
        cin >> mod_choise;   
        string sql = "INSERT INTO Classes (Name,Time,RoomID,Module_ID) "  \
        "VALUES ('"+class_name+"','"+time+"','"+to_string(choise)+"','"+to_string(mod_choise)+"'); ";
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
        break;
    }
    case 4:
    {
        string Room_Name;
        cout << "Enter the room name:";
        cin >> Room_Name;
        std::getline(cin,Room_Name);
        string sql = "INSERT INTO Rooms (Name) "  \
        "VALUES ('"+Room_Name+"'); ";

        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
        break;
    }
    case 5:
    {

        string Lecturers_Name;
        string time;
        int choise;
        int mod_choise;
        SQL_Data data_room = select_stmt("SELECT * FROM Rooms");
        
        if (data_room.empty())
        {
            cout << " \x1B[31m You need to add the Rooms first! \033[0m \n";
           break;
        }
        cout << "\x1B[32m Welcome to the Lecturers Menu\033[0m  \n\n";
        cout << "Write the name of the Lecturers\n\n";
        cin >> Lecturers_Name;
        std::getline(cin,Lecturers_Name);
        cout << "Write the time of the Lecturers (Format 00:00PM)\n\n";
        cin >> time;
        std::getline(cin,time);
        cout << "Select the Room \n\n";

        cout << "Choose the Room for the Lecturers \n";
        for (auto& SQLData : data_room) 
        {
       
            cout << SQLData[0] << ". " << SQLData[1] << "\n";
        }
        cin >> choise;     

        cout << "Choose the Module for the Lecturers \n";
        for (auto& SQLData : data) 
        {
           
            cout << SQLData[0] << ". " << SQLData[1] << "\n";
        }
        cin >> mod_choise;   
        string sql = "INSERT INTO Lecturers (Name,Time,RoomID,Module_ID) "  \
        "VALUES ('"+Lecturers_Name+"','"+time+"','"+to_string(choise)+"','"+to_string(mod_choise)+"'); ";
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
        break;
    }
    case 6:
    {
        int Student_opt;
        string student_name;
        SQL_Data Student_Data = select_stmt("SELECT * FROM Students");
        cout << "\x1B[32m Welcome to the Show Student Data Menu\033[0m  \n\n";
        cout << "Select the student you want to see the information:\n";
        for (auto& SQLData : Student_Data) 
        {
            cout << SQLData[0] << ". " << SQLData[1] << "\n";
        }
        cin >> Student_opt;
        student_name = Student_Data[Student_opt-1][1];
        string test = "SELECT Module_ID FROM Students Where ID = '"+to_string(Student_opt)+"'";
        SQL_Data Module_ID = select_stmt(test.c_str());
        vector<string> ModuleID = simple_tokenizer(Module_ID[0][0],",");

        cout << "Student Name : "+student_name+"";
        cout << "\nClasses : \n";
        for(auto var : ModuleID)
        {
            SQL_Data Student_Data  = select_stmt(("SELECT * FROM Classes WHERE Module_ID = "+var+"").c_str());
            if(Student_Data.empty()){ cout << "No more classes!"; break;}
            cout << "\t Name: " << Student_Data[0][1] << endl;
            cout << "\t Time: " << Student_Data[0][2] << endl;
            SQL_Data Room_Data  = select_stmt(("SELECT * FROM Rooms WHERE ID = "+Student_Data[0][3]+"").c_str());
            cout << "\t Room: \n" << Room_Data[0][1];
            

        }

        cout << "Lecturers : \n\n";
        for(auto var : ModuleID)
        {
            SQL_Data Student_Data  = select_stmt(("SELECT * FROM Lecturers WHERE Module_ID = "+var+"").c_str());
            if(Student_Data.empty()){ cout << "\n No more Lecturers! \n"; break;}
            cout << "\t Name: " << Student_Data[0][1] << endl;
            cout << "\t Time: " << Student_Data[0][2] << endl;
            SQL_Data Room_Data  = select_stmt(("SELECT * FROM Rooms WHERE ID = "+Student_Data[0][3]+"").c_str());
            cout << "\t Room: \n" << Room_Data[0][1];
            

        }

        break;
    }
    case 7:
        cout << "\x1B[31m YOU EXIT THE SOFTWARE SEE YOU NEXT TIME!\033[0m \n\n";
        exit(0);
        break;
    default:
        cout << "The option you selected isn't at the menu please select another one!";
       
        break;
   }

    Menu();
}
