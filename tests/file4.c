#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_SIZE            10
#define WRONG_SIZE_ARRAY    20
#define WRONG_ARRAY_ELEMENT 30
#define WRONG_TYPE_SIZE     40

int input_size_array(size_t *const size_array)
{
    int code_return = EXIT_SUCCESS;

    if (!scanf("%zu", size_array))
        code_return = WRONG_TYPE_SIZE;
    else if (*size_array > MAX_SIZE || *size_array < 1)
        code_return = WRONG_SIZE_ARRAY;
    
    return code_return;
}

int input_array(int *const array, size_t const size_array)
{
    int code_return = EXIT_SUCCESS;

    for (size_t i = 0; i < size_array; i++)
        if (scanf("%d", array + i) != 1)
            code_return = WRONG_ARRAY_ELEMENT;

    return code_return;
}

double count_mean_geometric(int *const array, size_t const size_array, double *mean_geometric)
{
    int counter = 0;
    int code_return = EXIT_SUCCESS;

    for (size_t i = 0; i < size_array; i++)
        if (array[i] > 0)
        {
            counter++;
            *mean_geometric *= array[i];
        }
    
    if (counter == 0)
        code_return = EXIT_FAILURE;
    else
        *mean_geometric = pow(*mean_geometric, 1.0 / counter);

    return code_return;
}


int main(void)
{
    int array[MAX_SIZE];
    size_t size_array;
    int code_return = input_size_array(&size_array);

    if (code_return)
        return code_return;
    
    code_return = input_array(array, size_array);
    
    if (code_return)
        return code_return;
    
    double mean_geometric = 1;
    code_return = count_mean_geometric(array, size_array, &mean_geometric);
    
    if (code_return)
        return code_return;

    printf("%lf\n", mean_geometric);

    return EXIT_SUCCESS;
}
