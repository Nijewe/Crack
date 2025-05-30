/**
*
* Author: Novice, Nijewe
* Title: Probabilistic 3-Digit Code Cracker
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//---------------------------------------------------------------------------------------
// STRUCT, TYPEDEF AND OTHER DEFINTION
//---------------------------------------------------------------------------------------

typedef struct Digit_state
{
    // Represents the probability of a digit appearing at each position in the code.

    bool leftpos;
    bool middlepos;
    bool rightpos;
}Digit_state_t;

typedef enum {SOLVED, UNSOLVED} Code_state_t;

//---------------------------------------------------------------------------------------
// GLOBAL VARIABLE
//---------------------------------------------------------------------------------------

Code_state_t code_state = UNSOLVED;

const int first_hint[]  = {6, 9, 0};
const int second_hint[] = {7, 4, 1};
const int third_hint[]  = {5, 0, 4};
const int fourth_hint[] = {3, 8, 7};
const int fiveth_hint[] = {2, 1, 9};

int cracked_code[3];    // Stores the final 3-digit cracked code (one digit per position)
Digit_state_t code_digit_state = {false, false, false}; // Represents the state of each position in the cracked code (found or not found)

// Dynamic array of digits that have a 100% probability of appearing in the code
int* correct_digit_array;
int correct_digit_count;

Digit_state_t all_digit_state[10];  // Matrix representing the probability of each digit appearing in each position of the code

//---------------------------------------------------------------------------------------
// FUNCTION PROTOTYPE
//---------------------------------------------------------------------------------------

void make_assumption(int digit_pos);

void one_number_is_correct_but_wrong_place(int* hint);
void two_numbers_are_correct_but_wrong_place(int* hint);
void nothing_is_correct(int* hint);

void update_state();

void clear_digit_state();
void clear_correct_number();
void add_correct_number(int digit);

void dump_state();

//---------------------------------------------------------------------------------------
// FUNCTION
//---------------------------------------------------------------------------------------

int main()
{
    for(int i = 0; i < 3 && code_state; i++)
    {
        code_digit_state.leftpos = false;
        code_digit_state.middlepos = false;
        code_digit_state.rightpos = false;

        clear_digit_state();
        clear_correct_number();
        add_correct_number(first_hint[i]);
        make_assumption(i);

        one_number_is_correct_but_wrong_place(second_hint);
        two_numbers_are_correct_but_wrong_place(third_hint);
        nothing_is_correct(fourth_hint);
        one_number_is_correct_but_wrong_place(fiveth_hint);

        // debugging purpose
        /*printf("-------------------------\n");

        dump_state();

        printf("-------------------------\n");*/

        update_state();
    }

    if(code_state == SOLVED)
    {
        printf("code craked successfully: ");
        for(int i = 0; i < 3; i++)
            printf("%d", cracked_code[i]);
    }
    else
        printf("unable to crack the code!");

    printf("\n");

    return 0;
}

void dump_state()
{
    for(int i = 0; i < 10; i++)
        printf("state of %d: (%d, %d, %d)\n", i, all_digit_state[i].leftpos, all_digit_state[i].middlepos, all_digit_state[i].rightpos);
}

void make_assumption(int digit_pos)
{
    for(int i = 0; i < 3; i++)
    {
        all_digit_state[first_hint[i]].leftpos     = false;
        all_digit_state[first_hint[i]].middlepos   = false;
        all_digit_state[first_hint[i]].rightpos    = false;

        if(i == digit_pos)
        {
            switch(digit_pos)
            {
            case 0:
                all_digit_state[first_hint[i]].leftpos = true;
                break;
            case 1:
                all_digit_state[first_hint[i]].middlepos = true;
                break;
            case 2:
                all_digit_state[first_hint[i]].rightpos  = true;
                break;
            }
        }
    }

    for(int i = 0; i < 10; i++)
    {
        if(i != first_hint[digit_pos])
        {
            switch(digit_pos)
            {
            case 0:
                all_digit_state[i].leftpos = false;
                break;
            case 1:
                all_digit_state[i].middlepos = false;
                break;
            case 2:
                all_digit_state[i].rightpos  = false;
                break;
            }
        }
    }

    // debugging purpose
    // printf("initial dump\n");
    // dump_state();
}

void update_state()
{
    for(int i = 0; i < 10; i++)
    {
        if(all_digit_state[i].leftpos && !code_digit_state.leftpos)
        {
            cracked_code[0] = i;
            code_digit_state.leftpos = true;
        }
        else
            if(all_digit_state[i].leftpos)
                return;

        if(all_digit_state[i].middlepos && !code_digit_state.middlepos)
        {
            cracked_code[1] = i;
            code_digit_state.middlepos = true;
        }
        else
            if(all_digit_state[i].middlepos)
                return;

        if(all_digit_state[i].rightpos && !code_digit_state.rightpos)
        {
            cracked_code[2] = i;
            code_digit_state.rightpos = true;
        }
        else
            if(all_digit_state[i].rightpos)
                return;
    }

    if(code_digit_state.rightpos && code_digit_state.leftpos && code_digit_state.middlepos)
            code_state = SOLVED;
}

void clear_digit_state()
{
    for (int i = 0; i < 10; i++)
    {
        all_digit_state[i].leftpos = true;
        all_digit_state[i].middlepos = true;
        all_digit_state[i].rightpos = true;
    }
}

void clear_correct_number()
{
    correct_digit_count = 0;
    if(correct_digit_array != NULL)
        free(correct_digit_array);

    correct_digit_array = NULL;
}

void add_correct_number(int digit)
{
    correct_digit_count++;

    correct_digit_array = realloc(correct_digit_array, sizeof(int) * correct_digit_count);
    correct_digit_array[correct_digit_count - 1] = digit;
}

bool is_digit_incorrect(int digit)
{
    return (!all_digit_state[digit].leftpos && !all_digit_state[digit].middlepos && !all_digit_state[digit].rightpos);
}

bool is_digit_correct(int digit)
{
    for(int i = 0; i < correct_digit_count; i++)
        if(correct_digit_array[i] == digit)
            return true;

    return false;
}

void one_number_is_correct_but_wrong_place(int* hint)
{
    for(int i = 0; i < 3; i++)
    {
        if(is_digit_correct(hint[i]))
        {
            for(int j = 0; j < 3; j++)
            {
                if(j != i)
                {
                    all_digit_state[hint[j]].leftpos     = false;
                    all_digit_state[hint[j]].middlepos   = false;
                    all_digit_state[hint[j]].rightpos    = false;
                }
            }

            return;
        }
    }

    for(int i = 0; i < 3; i++)
    {
        switch(i)
        {
        case 0:
            all_digit_state[hint[i]].leftpos = false;
            break;
        case 1:
            all_digit_state[hint[i]].middlepos = false;
            break;
        case 2:
            all_digit_state[hint[i]].rightpos  = false;
            break;
        }
    }

    int potential_correct_number;
    int incorrect_number_count = 0;
    for(int i = 0; i < 3; i++)
    {
        if(is_digit_incorrect(hint[i]))
            incorrect_number_count++;
        else
            potential_correct_number = hint[i];
    }

    if(incorrect_number_count == 2)
        add_correct_number(potential_correct_number);
}

void two_numbers_are_correct_but_wrong_place(int* hint)
{
    int correct_count = 0;
    for(int i = 0; i < 3; i++)
    {
        if(is_digit_correct(hint[i]))
        {
            correct_count++;

            if(correct_count == 2)
            {
                for(int j = 0; j < 3; j++)
                {
                    if(!is_digit_correct(hint[j]))
                    {
                        all_digit_state[hint[j]].leftpos     = false;
                        all_digit_state[hint[j]].middlepos   = false;
                        all_digit_state[hint[j]].rightpos    = false;
                    }
                }

                return;
            }

        }
    }

    for(int i = 0; i < 3; i++)
    {
        switch(i)
        {
        case 0:
            all_digit_state[hint[i]].leftpos = false;
            break;
        case 1:
            all_digit_state[hint[i]].middlepos = false;
            break;
        case 2:
            all_digit_state[hint[i]].rightpos  = false;
            break;
        }
    }

    int potential_incorrect_number_index;
    int incorrect_number_count = 0;
    for(int i = 0; i < 3; i++)
    {
        if(is_digit_incorrect(hint[i]))
        {
            incorrect_number_count++;
            potential_incorrect_number_index = i;
        }
    }

    if(incorrect_number_count == 1)
    {
        for(int i = 0; i < 3; i++)
            if(i != potential_incorrect_number_index)
                add_correct_number(hint[i]);
    }
}

void nothing_is_correct(int* hint)
{
    for(int i = 0; i < 3; i++)
    {
        all_digit_state[hint[i]].leftpos   = false;
        all_digit_state[hint[i]].middlepos = false;
        all_digit_state[hint[i]].rightpos  = false;
    }
}
