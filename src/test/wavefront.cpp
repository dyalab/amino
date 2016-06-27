#include "amino.h"
#include "amino/rx/wavefront.h"



int main(int argc, char **argv) {
    (void) argc; (void) argv;
    if( argc < 2 ) {
        fprintf(stderr, "Input not specified\n");
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    fprintf(stdout, "Input: %s\n", filename);
    aa_rx_wf_parse(filename);
    fprintf(stdout, "done\n");

    return 0;
}
