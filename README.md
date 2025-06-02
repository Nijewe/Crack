# Crack
A little c program for cracking a code based on some little information

# Overview

This program attempts to crack a 3-digit secret code by associating a probability score to each digit for each position (first, second, third).
Initially, all digits are considered equally likely to appear in any position. For instance:

- Digit 1 starts with a probability vector of (1, 1, 1)
- Digit 2 → (2, 2, 2)
- Digit 3 → (3, 3, 3)

As you can see, the initial assumption is absurd (every digit could be in every position), and the program will refine these probabilities based on the provided hints.

however I chose to only initialize digits that actually appear in the provided hints.
this helps immediately eliminate digits not present in any hint, reducing the
search space. While this approach may not be optimal for all puzzles (some may
expect unseen digits to still be considered), it works well for this specific logic.

# Processing Hints

Hints are given in the format:
"One digit is correct but in the wrong position"

For the input guess (1, 2, 3) and this hint, the program will reduce the probability of each digit being in its current position:

- 1 becomes: (-, 1, 1)
- 2 becomes: (2, -, 2)
- 3 becomes: (3, 3, -)

The program also performs optimizations.
For instance, if it's certain that some digits are not part of the code, they are eliminated completely (set to zero probability).
If digits 1 and 3 are ruled out, but the hint says "one digit is correct but in the wrong position,"
the program can conclude with certainty that digit 2 must be part of the solution.
Similarly, when a hint says "one digit is correct and in the correct position,"
the program cross-checks the digits in the guess with its internal list of digits known to be part of the code (those with 100% certainty).
If only one digit from the guess is present in this list,
it logically follows that the other two must be incorrect, they are then immediately assigned a zero probability across all positions.
This aggressive pruning helps reduce the search space significantly and accelerates convergence.

# Final Validation

After processing all hints, the program checks if:
- All three positions are assigned to exactly one digit
- There is no contradiction

Example of a contradiction:
If digit 1 has probability (1, -, -) and digit 3 has (3, -, -), both compete for the first position.
If all other possibilities are eliminated, the code is considered contradictory and unsolvable with the current assumptions.

# Limitations

This program is not designed to support general puzzle formats. It is tightly coupled to a specific type of clue especially at the begining:
"One digit is correct and well placed."
It uses this strict assumption to eliminate possibilities and converge on a solution quickly. It does not support such clues for general purpose clue

If you'd like to extend or adapt this logic, contributions or suggestions are welcome!