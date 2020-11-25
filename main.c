#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dict.h"

struct employee {
    char name[128];
    int age;
    int salary;
    int years;
};


int load_database(struct dict *D, const char *filename)
{
    FILE *fp;
    int ret;

    struct employee *item;

    if ( !(fp = fopen(filename, "r")) )
        return 1;

    while (1) {
        item = malloc(sizeof(*item));
        ret = fscanf(fp, "%[^,], %d, %d, %d\n",
            item->name, &item->age, &item->salary, &item->years);

        if (ret == EOF) {
            free(item);
            break;
        }

        dict_insert(D, item->name, item);
    }

    fclose(fp);

    return 0;
}


/* This function prints the provided struct person to the screen */
void print_employee(struct employee *p)
{
    printf("Employee %s:\n", p->name);
    printf("   Age: %d\n", p->age);
    printf("Salary: %d\n", p->salary);
    printf(" Years: %d\n", p->years);
}


void deleter(void *item)
{
    free(item);
}


int main(int argc, char **argv)
{
    int ret;
    struct dict team;
    struct employee *item;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s employee_name\n", argv[0]);
        return EXIT_FAILURE;
    }

    dict_init(&team, deleter);

    ret = load_database(&team, "database.txt");

    if (ret) {
        fprintf(stderr, "Failed to open database.\n");
        return EXIT_FAILURE;
    }

    item = dict_peek(&team, argv[1]);
    if (item) {
        print_employee(item);
        printf("\nThis employee has been deleted.\n");
        dict_delete(&team, argv[1]);
    } else {
        printf("Employee NOT FOUND!\n");
    }

    printf("\nLoad Factor: %0.2f\n", dict_loadfactor(&team));

    printf("Destroying Dictionary and exiting...\n");
    dict_destroy(&team);

    return 0;
}