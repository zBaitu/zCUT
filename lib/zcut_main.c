#include "zcut.h"

#include <stdlib.h>

int main(int argc, char* argv[])
{
    if (!ut_init(argc, argv))
      return EXIT_FAILURE;

    bool is_passed = ut_run();
    ut_fini();

    return (is_passed) ? EXIT_SUCCESS : EXIT_FAILURE;
}
