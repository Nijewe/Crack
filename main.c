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

#if 0
const int first_hint[]  = {6, 9, 0};
const int second_hint[] = {7, 4, 1};
const int third_hint[]  = {5, 0, 4};
const int fourth_hint[] = {3, 8, 7};
const int fiveth_hint[] = {2, 1, 9};
#elif 0
const int first_hint[]  = {2, 9, 1};
const int second_hint[] = {2, 4, 5};
const int third_hint[]  = {4, 6, 3};
const int fourth_hint[] = {5, 7, 8};
const int fiveth_hint[] = {5, 6, 9};
#else
const int first_hint[]  = {6, 8, 2};
const int second_hint[] = {6, 1, 4};
const int third_hint[]  = {2, 0, 6};
const int fourth_hint[] = {7, 3, 8};
const int fiveth_hint[] = {7, 8, 0};
#endif // 0

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

void one_number_is_correct_but_wrong_place(const int* hint);
void two_numbers_are_correct_but_wrong_place(const int* hint);
void nothing_is_correct(const int* hint);

void update_state();

void clear_digit_state();
void clear_correct_number();
void add_correct_number(int digit);

void calibrate_position();

void dump_state();

//---------------------------------------------------------------------------------------
// FUNCTION
//---------------------------------------------------------------------------------------

int main()
{
    for(int i = 0; i < 3 && code_state; i++)
    {
        // reset the state of each position in the cracked code
        code_digit_state.leftpos = false;
        code_digit_state.middlepos = false;
        code_digit_state.rightpos = false;

        clear_digit_state();    // reset the probability of each digit
        clear_correct_number(); // reset the dynamic array of all digit with 100% probability to appear

        /*
         * Based on the first hint, we make assumptions about each digit in the initial hint.
         * This allows us to immediately eliminate many other possibilities.
         * See the function `make_assumption()` for more details.
         */
        add_correct_number(first_hint[i]);  // we make the assumption that this digit is correct and well placed therefore
        make_assumption(i);                 // other digits in the hint are all wrong !

        // HINTS PROCESSING AFTER ASSUMPTION ...
        one_number_is_correct_but_wrong_place(second_hint);

        calibrate_position();

        two_numbers_are_correct_but_wrong_place(third_hint);

        calibrate_position();

        nothing_is_correct(fourth_hint);

        calibrate_position();

        one_number_is_correct_but_wrong_place(fiveth_hint);

        calibrate_position();

        // debugging purpose
        /*printf("-------------------------\n");

        dump_state();

        printf("-------------------------\n");*/

        // Updates the `code_state` based on the current probability matrix.
        // Also checks for contradictions See the function for detailed logic.
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

/*
 * Dumps the current probability state of all digits.
 * Used for debugging purposes to visualize internal state.
 */
void dump_state()
{
    for(int i = 0; i < 10; i++)
        printf("state of %d: (%d, %d, %d)\n", i, all_digit_state[i].leftpos, all_digit_state[i].middlepos, all_digit_state[i].rightpos);
}

/*
 * Sets the probability to zero for the specified position across all digits,
 * except for the given digit.
 */
void erase_position_except(int digit, int position)
{
    for(int i = 0; i < 10; i++)
    {
        if(i != digit)
        {
            switch(position)
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
}

/*
 * Making assumptions based on the first hint:
 * For each digit in the hint, assign zero probability to all digits in the specified position,
 * except for the digit at 'digit_pos', which retains its probability.
 *
 * This assumes that the digit at 'digit_pos' is correctly placed,
 * helping to eliminate other possibilities early.
 */
void make_assumption(int digit_pos)
{
    for(int i = 0; i < 3; i++)
    {
        // First, assign a probability of 0 to all digits from the first hint across all positions.
        all_digit_state[first_hint[i]].leftpos     = false;
        all_digit_state[first_hint[i]].middlepos   = false;
        all_digit_state[first_hint[i]].rightpos    = false;

        if(i == digit_pos)
        {
            /*
             * If we reach the digit that we assume is correct and well-placed,
             * we assign it a probability for its specific position hence assuming it's in the correct spot.
             */
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

    // Now that we've assumed this digit is correctly placed,
    erase_position_except(first_hint[digit_pos], digit_pos); // we assign zero probability to that position for all other digits.

    // debugging purpose
     //printf("initial dump\n");
     //dump_state();
}

/*
 * Check if the given digit has only one remaining possible position
 * based on its current probability map.
 */
bool is_digit_one_position_left(int digit, int* positionOut)
{
    int potential_position;
    int correct_pos_count = 0;
    if(all_digit_state[digit].leftpos)
    {
        correct_pos_count++;
        potential_position = 0;
    }

    if(all_digit_state[digit].middlepos)
    {
        correct_pos_count++;
        potential_position = 1;
    }

    if(all_digit_state[digit].rightpos)
    {
        correct_pos_count++;
        potential_position = 2;
    }

    if(correct_pos_count == 1)
    {
        *positionOut = potential_position;
        return true;
    }

    return false;
}

/*
 * For each digit marked as guaranteed to appear in the final code,
 * check if it has only one possible remaining position.
 * If so, set the probability of that position to zero for all other digits,
 * ensuring only that digit remains valid for that spot.
 */
void calibrate_position()
{
    int position;
    for(int i = 0; i < correct_digit_count; i++)
    {
        if(is_digit_one_position_left(correct_digit_array[i], &position))
            erase_position_except(correct_digit_array[i], position);
    }
}

/*
 * Check the final state of all digit probabilities to determine
 * whether exactly 3 positions have been resolved without conflicts.
 * Ensures that no two digits occupy the same position
 * and that all positions are filled.
 */
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
            if(all_digit_state[i].leftpos)  // if the position is availabe but
                return;                     // has been filled early it's contradiction !

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

    // If all digits have been successfully identified and their positions resolved,
    if(code_digit_state.rightpos && code_digit_state.leftpos && code_digit_state.middlepos)
            code_state = SOLVED;    // update the code_state
}

/*
 * This function resets the probabilities for all digits across all positions.
 *
 * Initially, the idea was to reset digits 0–9 so that each had equal probability
 * of appearing in any position. However, after further consideration, I chose to
 * only initialize digits that actually appear in the provided hints.
 *
 * This helps immediately eliminate digits not present in any hint, reducing the
 * search space. While this approach may not be optimal for all puzzles (some may
 * expect unseen digits to still be considered), it works well for this specific logic.
 */
void clear_digit_state()
{
    for(int i = 0; i < 3; i++)
    {
        all_digit_state[first_hint[i]].leftpos = true;
        all_digit_state[first_hint[i]].middlepos = true;
        all_digit_state[first_hint[i]].rightpos = true;
    }

    for(int i = 0; i < 3; i++)
    {
        all_digit_state[second_hint[i]].leftpos = true;
        all_digit_state[second_hint[i]].middlepos = true;
        all_digit_state[second_hint[i]].rightpos = true;
    }

    for(int i = 0; i < 3; i++)
    {
        all_digit_state[third_hint[i]].leftpos = true;
        all_digit_state[third_hint[i]].middlepos = true;
        all_digit_state[third_hint[i]].rightpos = true;
    }

    for(int i = 0; i < 3; i++)
    {
        all_digit_state[fourth_hint[i]].leftpos = true;
        all_digit_state[fourth_hint[i]].middlepos = true;
        all_digit_state[fourth_hint[i]].rightpos = true;
    }

    for(int i = 0; i < 3; i++)
    {
        all_digit_state[fiveth_hint[i]].leftpos = true;
        all_digit_state[fiveth_hint[i]].middlepos = true;
        all_digit_state[fiveth_hint[i]].rightpos = true;
    }
}

/*
 * Resets the table of digits that have a 100% probability of appearing in the code.
 * This is used to clear previous assumptions before processing new hints.
 */
void clear_correct_number()
{
    correct_digit_count = 0;
    if(correct_digit_array != NULL)
        free(correct_digit_array);

    correct_digit_array = NULL;
}

bool is_digit_correct(int digit)
{
    /*
     * A digit is considered correct if it appears in the list of digits
     * with 100% probability of being part of the final code.
     */
    for(int i = 0; i < correct_digit_count; i++)
        if(correct_digit_array[i] == digit)
            return true;

    return false;
}

void add_correct_number(int digit)
{
    if(is_digit_correct(digit))
        return; // no need to add !

    correct_digit_count++;

    correct_digit_array = realloc(correct_digit_array, sizeof(int) * correct_digit_count);
    correct_digit_array[correct_digit_count - 1] = digit;
}

bool is_digit_incorrect(int digit)
{
    /*
     * A digit is considered incorrect if its probabilities of appearing
     * in all positions are zero — meaning it cannot be part of the code.
     */
    return (!all_digit_state[digit].leftpos && !all_digit_state[digit].middlepos && !all_digit_state[digit].rightpos);
}

void one_number_is_correct_but_wrong_place(const int* hint)
{
    /*
     * When processing this hint, we start by assuming that each digit in the hint is correct,
     * but misplaced. This means that for each digit in the hint, we must set its probability
     * to zero for the position it currently occupies, since if it's indeed correct but in
     * the wrong place, then that position is certainly invalid for it.
     */
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

    /*
     * After that, we analyze the hints to detect eliminations.
     *
     * If two digits from a hint are determined to be incorrect (i.e., zero probability across all positions),
     * we can infer that the remaining digit must be part of the final code.
     *
     * That digit is then added to the list of confirmed digits (100% chance of appearing).
     */
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
    {
        add_correct_number(potential_correct_number);
        for(int i = 0; i < 3; i++)
            if(hint[i] != potential_correct_number)
            {
                all_digit_state[hint[i]].leftpos = false;
                all_digit_state[hint[i]].middlepos = false;
                all_digit_state[hint[i]].rightpos = false;
            }
    }

}

void two_numbers_are_correct_but_wrong_place(const int* hint)
{
    // see `one_number_is_correct_but_wrong_place()`
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

    /*
     * Unlike the logic in `one_number_is_correct_but_wrong_place()`, this function
     * tries to determine if only one digit in the hint is incorrect.
     *
     * If that's the case, we can infer that the remaining digits must be correct
     * and should therefore be added to the list of confirmed digits with 100%
     * certainty of appearing in the final code.
     */
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
        all_digit_state[hint[potential_incorrect_number_index]].leftpos = false;
        all_digit_state[hint[potential_incorrect_number_index]].middlepos = false;
        all_digit_state[hint[potential_incorrect_number_index]].rightpos = false;

        for(int i = 0; i < 3; i++)
            if(i != potential_incorrect_number_index)
                add_correct_number(hint[i]);
    }
}

void nothing_is_correct(const int* hint)
{
    /*
     * Here, we simply set the probability of these digits to zero across all positions,
     * since the hint confirms they are not part of the code.
     */
    for(int i = 0; i < 3; i++)
    {
        all_digit_state[hint[i]].leftpos   = false;
        all_digit_state[hint[i]].middlepos = false;
        all_digit_state[hint[i]].rightpos  = false;
    }
}