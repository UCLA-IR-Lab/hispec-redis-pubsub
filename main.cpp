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

        for (int i=0; i < 1000; i++) {
            string command("publish");
            command.append(" ");
            command.append("hsrtcsim");
            command.append(" ");
            
            string frame = "frame:" + to_string(i);
            command.append(frame);

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