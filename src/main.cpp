#include <crow.h>
#include <sqlite3.h>
#include <mutex>

#include <stdlib.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <string.h>

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
    const char *create_table = "CREATE TABLE IF NOT EXISTS emails (email text PRIMARY KEY NOT NULL);";
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
    
    CROW_ROUTE(TheShade, "/newsletter").methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        // Lock For Concurrency
        std::lock_guard<std::mutex> lock(concurrency_lock);

        // Load Email String
        auto email_json = crow::json::load(req.body);

        std::string email_string = email_json["email"].s();
        const char *email_char = email_string.c_str();

        int n = email_string.length();

        // Send Welcome Email
        pid_t pid;
	extern char **environ;


        char *argv[] = {"python", "../TheShadeNewsletter/newslettersignup.py", const_cast<char*>(email_string.c_str()), NULL};

        // Spawn Newsletter Python Script
        int status = posix_spawn(&pid, "../TheShadeNewsletter/TheShadeVenv/bin/python", NULL, NULL, argv, environ);

        if (status != 0) {
            return crow::response(500);
        }

        sqlite3_stmt *stmt;
        const char *email_insert = "INSERT INTO emails (email) VALUES (?);";

        int email_rc;

        email_rc = sqlite3_prepare_v2(email_db, email_insert, -1, &stmt, nullptr);

        sqlite3_bind_text(stmt, 1, email_char, -1, SQLITE_TRANSIENT);

        email_rc = sqlite3_step(stmt);

        if (email_rc != SQLITE_DONE) {
            std::cerr << "Insert failed: " << sqlite3_errmsg(email_db) << std::endl;
        }

        return crow::response(200);
    });

    TheShade.bindaddr("127.0.0.1").port(18080).multithreaded().run();
}
