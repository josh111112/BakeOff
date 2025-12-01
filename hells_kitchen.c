/*
 Global resources:
 Mixers:            2
 Pantry:            1
 Fridges:           2
 Bowls:             3
 Spoons:            5
 Oven:              1
 */

/*
 Data structure to hold ingredients list for:
 Cookies:                   Flour, Sugar, Milk, Butter
 Pancakes:                  Flour, Sugar, Baking soda, Salt, Egg, Milk, Butter
 Homemade pizza dough:      Yeast, Sugar, Salt
 Soft Pretzels:             Flour, Sugar, Salt, Yeast, Baking Soda, Egg
 Cinnamon rolls:            Flour, Sugar, Salt, Butter, Eggs, Cinnamon
 */

/*
 Baker struct:
 BakerID (track who is doing what)
 Color code
 Ramsied flag
 */

/*
 Counting semaphores:   Initlialize to count of specific resources (global resources)
 Mutexes:               Initiliaze to 1 for resources that allow single access like Pantry and Oven, Fridge 1 and Fridge 2
 */

/*
 Baker must iterate throught every recipe on the list
 Ingredient Aquisition:
 Acquire Pantry mutex to get dry ingredients:               (1. Flour 2. Sugar 3. Yeast 4. Baking Soda 5. Salt 6. Cinnamon)
 Release Pantry mutex
 Acquire Fridge1 or Fridge2 mutex to get wet ingredients:   (1. Egg(s) 2. Milk 3. Butter)
 Release Fridge mutex
 Baker must only access one ingredient at a time
 */

/*
 Ramsied Logic:
 Randomly check if current baker is the designated candidate to be ramsied
 If Ramsied, release all held semaphores and restart
 */

/*
 Tool logic:
 When all ingredients are secured, baker must acquire: Bowl, Spoon, and Mixer
    Careful for deadlock, order matters, one could be holding spoon and waiting for bowl, while other holds bowl and waits for spoon
 */

/*
 Baking logic:
 Mix ingredients
 Acquire Oven Semapore
 "Cook"
 Release Oven semaphore
 */

/*
 Cleanup:
 Release all tool semaphores (Bowl, Spoon, Mixer)
 */

/*
 Output & Main Execution:
 Take user input for number of bakers
 Spawn a thread for each baker
 Every major action prints to the terminal
 Implement ANSI color codes so each baker's text is distinct
 */
