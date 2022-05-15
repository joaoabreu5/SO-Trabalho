typedef struct operation
{
    int nop, bcompress, bdecompress, gcompress, gdecompress, encrypt, decrypt;
} operation, *Operation;

Operation parse(int);
void print_Op(Operation);