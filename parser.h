typedef struct operation
{
    int n_max;
    char *op;
} operation;

typedef struct operation *Operation;

Operation parse(char *filename);
int free_Operation(Operation opv);
int get_nr_max(char *op1, Operation opv);