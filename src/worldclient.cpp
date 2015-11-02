#include "worldclient.h"

int main(int argc, char *argv[])
{
    Options opts;
    parse_args(argc, argv, &opts);
    NCursesClient nc_client(opts.pipe);
    nc_client.print_tanks();
    return 0;
}

void parse_args(int argc, char *argv[], Options * opts)
{
    if (argc < 2) {
        cout << "Wrong argument" << endl;
        print_help(argv[0]);
        exit(-1);
    }
    struct option longopts[] = {
        { "pipe", required_argument, NULL, 'p' },
        { "help", no_argument,       NULL, 'h' },
        { 0, 0, 0, 0 }
    };
    int c;
    while ( (c = getopt_long(argc, argv, "p:h", longopts, NULL) ) != -1) {
        switch (c) {
        case 'p':
            opts->pipe = optarg;
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
}

void print_help(char * progname)
{
    cout << "Usage: " << progname << " [-p <path to named pipe> | -h]" << endl;
    cout << endl;
}
