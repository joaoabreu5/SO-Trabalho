typedef struct operation
{
    int nop, bcompress, bdecompress, gcompress, gdecompress, encrypt, decrypt;
} operation;

typedef struct operation *Operation;

Operation parse(int);
void print_Op(Operation);