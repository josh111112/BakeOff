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


/* ===============================================================================
 ACTION ITEM LIST (TODO)
 ===============================================================================
 
 2. [ ] DEFINE GLOBAL SEMAPHORES:
    - We need to declare the actual semaphore variables at the top of the file
      so all threads can see them.
    - sem_t mixer_sem;   (Count: 2)
    - sem_t pantry_sem;  (Count: 1 - Binary/Mutex)
    - sem_t fridge_sem[2]; (Count: 1 each - Two separate mutexes)
    - sem_t bowl_sem;    (Count: 3)
    - sem_t spoon_sem;   (Count: 5)
    - sem_t oven_sem;    (Count: 1 - Binary/Mutex)

 PART 2: INITIALIZATION
 -------------------------------------------------------------------------------
 3. [ ] INITIALIZE SEMAPHORES IN MAIN:
    - Before creating threads, use sem_init() for all the variables above.
    - Example: sem_init(&mixer_sem, 0, 2);

 PART 3: HELPER FUNCTIONS (To keep thread code clean)
 -------------------------------------------------------------------------------
 4. [ ] CREATE "get_pantry_ingredients(int baker_id, Recipe* r)"
    - Logic: sem_wait(pantry) -> loop through pantry ingredients -> print output -> sem_post(pantry).
    - Remember: Only one baker in pantry at a time.
 
 5. [ ] CREATE "get_fridge_ingredients(int baker_id, Recipe* r)"
    - Logic: Try to access Fridge 1. If busy, try Fridge 2.
    - Use sem_trywait() or logic to pick a free fridge.
    - Remember: Baker A can be in Fridge 1 while Baker B is in Fridge 2.

 6. [ ] CREATE "get_tools_and_mix(int baker_id)"
    - Logic: Acquire Bowl, Spoon, Mixer.
    - CRITICAL: Define a strict order (e.g., Always Bowl -> then Spoon -> then Mixer) to prevent Deadlock.
 
 PART 4: THE THREAD LOOP (The Core Logic)
 -------------------------------------------------------------------------------
 7. [ ] IMPLEMENT "void *baker_routine(void *arg)"
    - Cast arg back to BakerInfo*.
    - Loop through all 5 recipes (using the global array from Part 1).
    - Inside the loop:
      a. Check for "Ramsied" status (Random chance if baker->ramsieAble is 1).
      b. Call get_pantry_ingredients().
      c. Call get_fridge_ingredients().
      d. Call get_tools_and_mix().
      e. sem_wait(oven) -> Bake -> sem_post(oven).
      f. Release tools (sem_post mixer/bowl/spoon).

 PART 5: EXECUTION & CLEANUP
 -------------------------------------------------------------------------------
 8. [ ] SPAWN THREADS IN MAIN:
    - Loop num_bakers times.
    - Create BakerInfo struct for each (malloc it).
    - Assign colors (Use a simple array of string codes).
    - Select one baker to set ramsieAble = 1.
    - pthread_create().

 9. [ ] CLEANUP:
    - pthread_join() all threads.
    - Free recipe memory.
    - Destroy semaphores.
 ===============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// structs
typedef struct {
    int baker_id;
    char *color_code;
    int ramsieAble;
} BakerInfo;

typedef struct {
    int required_pantry[6];
    int required_fridge[3];
} Recipe;

enum RecipeName { COOKIES, PANCAKES, PIZZA, PRETZELS, CINNAMON_ROLLS };
enum PantryIng { FLOUR, SUGAR, YEAST, BAKING_SODA, SALT, CINNAMON };
enum FridgeIng { EGGS, MILK, BUTTER };

Recipe *all_recipes[5];


int initializeRecipes() {
    // RECIPE 0: COOKIES
    // Ingredients: Flour, Sugar, Milk, Butter
    all_recipes[COOKIES] = malloc(sizeof(Recipe));
    // Initialize all counts to 0 first to avoid garbage values
    for(int i = 0; i < 6; i++) all_recipes[COOKIES]->required_pantry[i] = 0;
    for(int i = 0; i < 3; i++) all_recipes[COOKIES]->required_fridge[i] = 0;
    
    // Set Ingredients
    all_recipes[COOKIES]->required_pantry[FLOUR] = 1;
    all_recipes[COOKIES]->required_pantry[SUGAR] = 1;
    all_recipes[COOKIES]->required_fridge[MILK] = 1;
    all_recipes[COOKIES]->required_fridge[BUTTER] = 1;

    // RECIPE 1: PANCAKES
    // Ingredients: Flour, Sugar, Baking soda, Salt, Egg, Milk, Butter
    all_recipes[PANCAKES] = malloc(sizeof(Recipe));
    for(int i = 0; i < 6; i++) all_recipes[PANCAKES]->required_pantry[i] = 0;
    for(int i = 0; i < 3; i++) all_recipes[PANCAKES]->required_fridge[i] = 0;

    all_recipes[PANCAKES]->required_pantry[FLOUR] = 1;
    all_recipes[PANCAKES]->required_pantry[SUGAR] = 1;
    all_recipes[PANCAKES]->required_pantry[BAKING_SODA] = 1;
    all_recipes[PANCAKES]->required_pantry[SALT] = 1;
    all_recipes[PANCAKES]->required_fridge[EGGS] = 1;
    all_recipes[PANCAKES]->required_fridge[MILK] = 1;
    all_recipes[PANCAKES]->required_fridge[BUTTER] = 1;

    // RECIPE 2: HOMEMADE PIZZA DOUGH
    // Ingredients: Yeast, Sugar, Salt
    all_recipes[PIZZA] = malloc(sizeof(Recipe));
    for(int i = 0; i < 6; i++) all_recipes[PIZZA]->required_pantry[i] = 0;
    for(int i = 0; i < 3; i++) all_recipes[PIZZA]->required_fridge[i] = 0;

    all_recipes[PIZZA]->required_pantry[YEAST] = 1;
    all_recipes[PIZZA]->required_pantry[SUGAR] = 1;
    all_recipes[PIZZA]->required_pantry[SALT] = 1;

    // RECIPE 3: SOFT PRETZELS
    // Ingredients: Flour, Sugar, Salt, Yeast, Baking Soda, Egg
    all_recipes[PRETZELS] = malloc(sizeof(Recipe));
    for(int i = 0; i < 6; i++) all_recipes[PRETZELS]->required_pantry[i] = 0;
    for(int i = 0; i < 3; i++) all_recipes[PRETZELS]->required_fridge[i] = 0;

    all_recipes[PRETZELS]->required_pantry[FLOUR] = 1;
    all_recipes[PRETZELS]->required_pantry[SUGAR] = 1;
    all_recipes[PRETZELS]->required_pantry[SALT] = 1;
    all_recipes[PRETZELS]->required_pantry[YEAST] = 1;
    all_recipes[PRETZELS]->required_pantry[BAKING_SODA] = 1;
    all_recipes[PRETZELS]->required_fridge[EGGS] = 1;

    // RECIPE 4: CINNAMON ROLLS
    // Ingredients: Flour, Sugar, Salt, Butter, Eggs, Cinnamon
    all_recipes[CINNAMON_ROLLS] = malloc(sizeof(Recipe));
    for(int i = 0; i < 6; i++) all_recipes[CINNAMON_ROLLS]->required_pantry[i] = 0;
    for(int i = 0; i < 3; i++) all_recipes[CINNAMON_ROLLS]->required_fridge[i] = 0;

    all_recipes[CINNAMON_ROLLS]->required_pantry[FLOUR] = 1;
    all_recipes[CINNAMON_ROLLS]->required_pantry[SUGAR] = 1;
    all_recipes[CINNAMON_ROLLS]->required_pantry[SALT] = 1;
    all_recipes[CINNAMON_ROLLS]->required_pantry[CINNAMON] = 1;
    all_recipes[CINNAMON_ROLLS]->required_fridge[BUTTER] = 1;
    all_recipes[CINNAMON_ROLLS]->required_fridge[EGGS] = 1;

    return 0;
}

int main(){
    int num_bakers;
    
    printf("Enter the number of bakers: \n");
    
    if (scanf("%d", &num_bakers) != 1) {
        fprintf(stderr, "Error: Invalid input. Please enter a number.\n");
        return 1;
    }
    initializeRecipes();
    printf("Starting Bake Off with %d bakers...\n", num_bakers);
    
    return 0;
}



