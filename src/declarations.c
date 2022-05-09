#include <stdio.h>

int number_of_Digits(int x)
{
    int count = 0;
    if (x < 0)
    {
        count++;
    }
    do
    {
        x /= 10;
        count++;
    } while (x != 0);
    return count;
}