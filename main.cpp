#include <bits/stdc++.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

using namespace std;

void archonSimulator(redisAsyncContext *c, void *r, void *privdata);
void ttmSimulator(redisAsyncContext *c, void *r, void *privdata);

int main(int argc, char* argv[]) {
    cout << "HISPEC Redis Pub/Sub Demo Code" << endl;

    signal(SIGPIPE, SIG_IGN);
    struct event_base* base = event_base_new();

    if (argv[optind] == NULL) {
        cout << "Mandatory argument missing" << endl;
        cout << "Usage: [-d {archon,ttm}] [-v]" << endl;
        cout << "Exiting the program..." << endl;
        return 1;
    }

    int opt;
    string deviceName;
    
    opterr = 0;

    while ((opt = getopt(argc, argv, "vd:")) != -1) {
        switch (opt) {
            case 'v':
                cout << "Verbose mode set" << endl;
                break;
            
            case 'd':
                cout << "Device: " << optarg << endl;
                deviceName = optarg;
                break;

            default:
                cout << "Usage: [-d {archon,ttm}] [-v]" << endl;
                break;
        }
    }

    redisAsyncContext* rc = redisAsyncConnect("127.0.0.1", 6379);
    if (rc->err) {
        cout << "Redis Connection Error: " << rc->errstr << endl;
        return 1;
    }
    int ra = redisAsyncCommand(rc, NULL, NULL, "AUTH %s", "hispec");
    if (ra != REDIS_OK) {
        cout << "Redis Authentication Error" << endl;
        return 1;
    }
    redisLibeventAttach(rc, base);

    int status;
    if (deviceName.compare("archon") == 0) {

        for (int i=0; i < 100; i++) {
            string operation("publish");
            string channel("hsrtcsim");
            // string data("archon_test");

            vector<uint32_t> v(10, 0);
            std::mt19937 mt{std::random_device{}()};
            std::uniform_int_distribution<uint32_t> dist(0, numeric_limits<uint32_t>::max());
            for (int i=0; i < v.size(); i++) {
                v[i] = dist(mt);
            }
            string data(v.begin(), v.end());

            // string data = to_string(dist(mt));
            ostringstream msg;
            msg << operation << " " << channel << " " << data;

            string command = msg.str();
            cout << command << endl;

            status = redisAsyncCommand(rc, archonSimulator, (char*)"pub", command.c_str());
        }
    } else if (deviceName.compare("ttm") == 0) {
        string command("subscribe");
        command.append(" ");
        command.append("hsrtcsim");

        status = redisAsyncCommand(rc, ttmSimulator, (char*)"sub", command.c_str());
    } else {
        cout << "Choose archon or ttm" << endl;
    }

    event_base_dispatch(base);

    return 0;
}

void archonSimulator(redisAsyncContext *c, void *r, void *privdata) {
    redisReply* reply = (redisReply*)r;
    if (reply == NULL) {
        cout << "Redis Publishing Error" << endl;
        return;
    }
    cout << "Message published" << endl;
    redisAsyncDisconnect(c);
    usleep(1000);
}

void ttmSimulator(redisAsyncContext *c, void *r, void *privdata) {
    redisReply* reply = (redisReply*)r;
    if (reply == NULL) {
        cout << "Redis Subscribing Error" << endl;
        return;
    }
    if ((reply->type == REDIS_REPLY_ARRAY) & (reply->elements == 3)) {
        if (strcmp(reply->element[0]->str, "subscribe") != 0) {
            cout << "Archon 10x10 Frame: " << reply->element[2]->str << endl;
        }
    }
}