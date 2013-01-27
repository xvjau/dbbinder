#include "select.h"
#include "insert.h"

#include <iostream>

using namespace std;

int main()
{
    MYSQL *m_conn = nullptr;

    {
        unsigned int arg = 1;
        my_bool val = true;

        MYSQL *conn = mysql_init(nullptr);
        mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &arg);
        mysql_options(conn, MYSQL_OPT_RECONNECT, &val);

        m_conn = mysql_real_connect(conn, "localhost", "root", "masterkey", "test", 3306, NULL, 0);
    }

    {
        insert_blob ins(m_conn);

        const char* str = "Hey there!  Another test is here.";

        shared_ptr<vector<char>> blob = make_shared<vector<char>>(str, str + strlen(str));

        ins.insert(blob);
    }

    {
        select_blob sb(m_conn);

        sb.open();

        for(auto i : sb)
        {
            auto comment = i->getcomment();
            string str;
            if (comment->size())
                str.assign(&((*comment)[0]), comment->size());

            cout << "Id: " << i->getid() << "\n";
            cout << "Comment: '" <<  str << "'\n";
            cout << endl;
        }
    }

    mysql_close(m_conn);
}