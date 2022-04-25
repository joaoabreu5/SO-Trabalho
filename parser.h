typedef struct operation
{
    int nop, bcompress, bdecompress, gcompress, gdecompress, encrypt, decrypt;
} operation;

typedef struct operation *Operation;

Operation parse(int fd);
void print_Op(Operation op);