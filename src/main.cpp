#include <crow.h>
#include <sqlite3.h>
#include <mutex>

int main()
{
    // Initialize DB
    sqlite3 *email_db;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("emails.db", &email_db);

    if (rc) {
        fprintf(stderr, "Cant Open database: %s\n", sqlite3_errmsg(email_db));
        return 0;
    }

    // Create Emails Table
    sqlite3_stmt *create_table_result;
    const char *create_table = "CREATE TABLE IF NOT EXISTS emails (emails text PRIMARY KEY NOT NULL);";
    int statement_size = sizeof(create_table);

    rc = sqlite3_exec(email_db, create_table, NULL, NULL, &zErrMsg);

    if (rc) {
        fprintf(stderr, "Error In Creating Emails Table %s\n", zErrMsg);
    }

    // Initialize App
    crow::SimpleApp TheShade;
    
    // Concurrency Lock
    std::mutex concurrency_lock;

    CROW_ROUTE(TheShade, "/")([](){
        return "Hello world";
    });

    
    CROW_ROUTE(TheShade, "/newsletter")
    ([](const crow::request& req){
        // TODO: Add Lock For Concurrency

        // Load 
        auto email_json = crow::json::load(req.body);


        return "Hello world";
    });

    TheShade.port(18080).multithreaded().run();
}