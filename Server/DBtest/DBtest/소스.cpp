#include <windows.h> 
#include <sqlext.h> 
#include <iostream> 
#include <chrono>
#include <stdio.h>

using namespace std;
using namespace chrono;

#define MAX_NAME 16

int main()
{
    system_clock::time_point start;

    char user_id[MAX_NAME];
    char buf[100];
    int user_time;

    // Connect to DB
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt = 0;
    SQLRETURN retcode;

    SQLWCHAR szUser_Name[MAX_NAME];
    SQLINTEGER dUser_time, dUser_level;



    setlocale(LC_ALL, "korean");

    SQLLEN cbName = 0, cbID = 0, cbLevel = 0;

    SQLCHAR* OutConnStr = (SQLCHAR*)malloc(255);
    SQLSMALLINT* OutConnStrLen = (SQLSMALLINT*)malloc(255);

    // Allocate environment handle  
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

    // Set the ODBC version environment attribute  
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

        // Allocate connection handle  
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

            // Set login timeout to 5 seconds  
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

                // Connect to data source  
                retcode = SQLConnect(hdbc, (SQLCHAR*)"veles_db_odbc", SQL_NTS, (SQLCHAR*)NULL, 0, NULL, 0);

                // Allocate statement handle  
                if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

                    cout << "ODBC connect success " << endl;

                    cout << "User ID: " << endl;
                    cin >> user_id;
                    cout << "User Time: " << endl;
                    cin >> user_time;

                    sprintf_s(buf, sizeof(buf), "EXEC update_time %s, %d", user_id, user_time);
                    

                    retcode = SQLExecDirect(hstmt, (SQLCHAR*) buf, SQL_NTS);

                    cout << "retcode: " << retcode << endl;
                    //swprintf_s(buf, L"EXEC select_time %s", user_id);
                    //retcode = SQLExecDirect(hstmt, buf, SQL_NTS);

                    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

                        cout << "EXEC SUCCESS" << endl;

                        //retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &dUser_time, 100, &cbID);

                        //retcode = SQLFetch(hstmt);
                        //if (retcode == SQL_ERROR)
                        //{
                        //    cout << "error: " << retcode << endl;
                        //}

                        //wprintf(L"%d: %d \n", user_id, dUser_time);
                    }
                    else {
                        cout << "EXEC FAILED" << endl;
                    }
                }
                //

                // Process data  
                if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                    SQLCancel(hstmt);
                    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
                }

                SQLDisconnect(hdbc);
            }

            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        }
    }
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
    system("pause");
}

//void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode) {
//    // 에러 확인 함수
//
//    SQLSMALLINT iRec = 0; 
//    SQLINTEGER iError; 
//    WCHAR wszMessage[1000]; 
//    WCHAR wszState[SQL_SQLSTATE_SIZE + 1]; 
//    if (RetCode == SQL_INVALID_HANDLE) { 
//        fwprintf(stderr, L"Invalid handle!\n"); 
//        return;
//    } 
//    while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage, (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS)
//    {
//        // Hide data truncated.. 
//        if (wcsncmp(wszState, L"01004", 5)) {
//            fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
//        }
//    }
//}

