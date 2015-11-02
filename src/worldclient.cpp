#include "worldclient.h"

int main(int argc, char *argv[])
{
    char * pipe = parse_args(argc, argv);
    return 0;
}

char * parse_args(int argc, char *argv[])
{
    struct option longopts[] = {
        { "pipe", required_argument, NULL, 'p' },
        { "help", no_argument,       NULL, 'h' },
        { 0, 0, 0, 0 }
    };
    int c;
    char * pipe = nullptr;
    while ( (c = getopt_long(argc, argv, "p:h", longopts, NULL) ) != -1) {
        switch (c) {
        case 'p':
            pipe = optarg;
            break;
        case 'h':
            print_help(argv[0]);
            exit(0);
            break;
        default:
            cout << "Wrong argument" << endl;
            print_help(argv[0]);
            exit(-1);
        }
    }
    return pipe;
}

void print_help(char * progname)
{
    cout << "Usage: " << progname << " [-p <path to named pipe> | -h]" << endl;
    cout << endl;
}
