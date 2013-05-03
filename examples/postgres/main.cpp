#include <select.h>

#include <iostream>

using namespace std;

PGconn* connect()
{
    std::vector<const char*> keywords = {
        "host",
        "port",
        "dbname",
        "user",
        "password",
        "connect_timeout",
        "client_encoding",
        "application_name",
        "keepalives",
        nullptr
    };
    std::vector<const char*> values = {
        "localhost",
        "5432",
        "test",
        "postgres",
        "masterkey",
        "60",
        "utf8",
        "cppplay",
        "1",
        nullptr
    };

    auto result = PQconnectdbParams(keywords.data(), values.data(), 0);

    ConnStatusType status = PQstatus(result);
    if(status != CONNECTION_OK)
    {
        std::cerr << "PostgreSQL connect error: " << PQerrorMessage(result) << std::endl;
        exit(1);
    }
    else
    {
        PQsetSingleRowMode(result);
    }

    return result;
}

int main()
{
    PGconn *conn = connect();

    select_blob sb(conn);

    sb.open("Hello");

    for(auto i : sb)
    {
        std::string name;
        if (!i->isNullname())
            name = i->getname();

        cout << "Id: " << i->getid() << "\n";
        cout << "Name: '" <<  name << "'\n";
        cout << endl;
    }

}
