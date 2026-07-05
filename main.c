#include "codex.h"

int main(int ac, char **av)
{
    t_args var;

    if (ac != 9)
    {
        printf("The number of argument should be 8.\n");
        return 0;
    }
    if (args_to_struct(ac, av, &var) == -1)
    {
        printf("Invalid arguments.\n");
        return 0;
    }


    printf("this: %ld\n", var.number_of_coders);
    printf("this: %ld\n", var.time_to_burnout);
    printf("this: %ld\n", var.time_to_compile);
    printf("this: %ld\n", var.time_to_debug);
    printf("this: %ld\n", var.time_to_refactor);
    printf("this: %ld\n", var.number_of_compiles_required);
    printf("this: %ld\n", var.dongle_cooldown);
    printf("this: %s\n", var.scheduler);
}