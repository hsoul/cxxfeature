#include "algorithm_test.h"
#include "coroutine_unitls_test.h"
#include "coroutines_test.h"
#include "grammar_test.h"
#include "std_test.h"
#include <bitset>
#include <chrono>
#include <getopt.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <unistd.h>

int parse_opt(int argc, char** argv)
{
    int oc; /* option character */
    char* b_opt_arg;

    while ((oc = getopt(argc, argv, ":ab:")) != -1) // a 不需要参数， b 一定需要参数;在optstring参数开头第一个字符为 : 时，遇到参数缺失返回:
    {
        switch (oc)
        {
            case 'a':
                /* handle -a, set a flag, whatever */
                printf("-a\n");
                break;
            case 'b':
                /* handle -b, get arg value from optarg */
                b_opt_arg = optarg;
                printf("-b %s\n", b_opt_arg);
                break;
            case ':':
                /* missing option argument 参数缺失*/
                fprintf(stderr, "%s: option '-%c' requires an argument\n", argv[0], optopt);
                break;
            case '?':
            default:
                /* invalid option 非法选项*/
                fprintf(stderr, "%s: option '-%c' is invalid: ignored\n", argv[0], optopt);
                break;
        }
    }
    return 0;
}

int parse_long_opt(int argc, char** argv)
{
    int c;
    int digit_optind = 0;

    while (1)
    {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;

        int add_c = 0;

        static struct option long_options[] = {
            {"add",     required_argument, &add_c, 17 },
            {"append",  no_argument,       0,      0  },
            {"delete",  required_argument, 0,      0  },
            {"verbose", no_argument,       0,      0  },
            {"create",  required_argument, 0,      'c'},
            {"file",    required_argument, 0,      0  },
            {0,         0,                 0,      0  }
        };

        c = getopt_long(argc, argv, "abc:d:012", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s\n", optarg);
                if (::strcmp(long_options[option_index].name, "add") == 0)
                {
                    printf("add_c is %d\n", add_c);
                }
                printf("\n");
                break;

            case '0':
            case '1':
            case '2':
                if (digit_optind != 0 && digit_optind != this_option_optind)
                    printf("digits occur in two different argv-elements.\n");
                digit_optind = this_option_optind;
                printf("option %c\n", c);
                break;

            case 'a':
                printf("option a\n");
                break;

            case 'b':
                printf("option b\n");
                break;

            case 'c':
                printf("option c with value '%s'\n", optarg);
                break;

            case 'd':
                printf("option d with value '%s'\n", optarg);
                break;

            case '?':
                break;

            default:
                printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (optind < argc)
    {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

    exit(EXIT_SUCCESS);

    return 0;
}

int main(int argc, char** argv)
{
    // parse_opt(argc, argv);
    // parse_long_opt(argc, argv);
    // Test::DoCoroutinesTest();
    // Test::DoCoroutineUnitlsTest();
    // Test::DoAlgorithmTest();
    Test::DoGrammarTest();
    // Test::DoStdTest();

    return 0;
}
