#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// NOTE: will not run on Windows, this uses POSIX

// ----------------------------------------------------------------------------------------- GLOBALS ----------------------------------------------------------------------------------------
// structs
typedef struct {
    int required_pantry[6];
    int required_fridge[3];
} Recipe;

typedef struct {
    int baker_id;
    char *color_code;
    int ramsieAble;
    int recipes_completed;
    Recipe *current_recipe;
} BakerInfo;

// enums
enum RecipeName { COOKIES, PANCAKES, PIZZA, PRETZELS, CINNAMON_ROLLS };
enum PantryIng { FLOUR, SUGAR, YEAST, BAKING_SODA, SALT, CINNAMON };
enum FridgeIng { EGGS, MILK, BUTTER };

// lists
const char *fridge_names[] = {"Eggs", "Milk", "Butter"};
const char *pantry_names[] = {"Flour", "Sugar", "Yeast", "Baking Soda", "Salt", "Cinnamon"};
const char *recipe_names[] = {"Cookies", "Pancakes", "Homemade Pizza", "Soft Pretzels", "Cinnamon Rolls"};

sem_t *mixer_sem;
sem_t *pantry_sem;
sem_t *fridge_sem[2];
sem_t *bowl_sem;
sem_t *spoon_sem;
sem_t *oven_sem;

Recipe *all_recipes[5];


// ----------------------------------------------------------------------------------- INITIALIZER FUNCTIONS -------------------------------------------------------------------------------
int initializeRecipes() {
    // RECIPE 0: COOKIES
    // Ingredients: Flour, Sugar, Milk, Butter
    all_recipes[COOKIES] = malloc(sizeof(Recipe));
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

int initializeSemaphores() {
    sem_unlink("/mixer");
    sem_unlink("/pantry");
    sem_unlink("/fridge0");
    sem_unlink("/fridge1");
    sem_unlink("/bowl");
    sem_unlink("/spoon");
    sem_unlink("/oven");

    mixer_sem = sem_open("/mixer", O_CREAT, 0644, 2);
    pantry_sem = sem_open("/pantry", O_CREAT, 0644, 1);
    fridge_sem[0] = sem_open("/fridge0", O_CREAT, 0644, 1);
    fridge_sem[1] = sem_open("/fridge1", O_CREAT, 0644, 1);
    bowl_sem = sem_open("/bowl", O_CREAT, 0644, 3);
    spoon_sem = sem_open("/spoon", O_CREAT, 0644, 5);
    oven_sem = sem_open("/oven", O_CREAT, 0644, 1);
    return 0;
}

// ----------------------------------------------------------------------------------- HELPER FUNCTIONS -------------------------------------------------------------------------------
int getPantryIngredients(BakerInfo* baker, Recipe* r) {
    sem_wait(pantry_sem);
    printf("%sBaker %d is accessing the pantry...%s\n",
               baker->color_code,
               baker->baker_id,
               "\033[0m");

    for (int i = 0; i < 6; i++) {
        // Check if the recipe requires this ingredient
        if (r->required_pantry[i] == 1) {
            
            printf("%sBaker %d grabbed %s%s\n",
                   baker->color_code,
                   baker->baker_id,
                   pantry_names[i],
                   "\033[0m");
                   
            usleep(100000);
        }
    }
    sem_post(pantry_sem);
    return 0;
}

int getFridgeIngredients(BakerInfo* baker, Recipe* r) {
    // fridge choice logic
    int fridge_id = -1;
    if (sem_trywait(fridge_sem[0]) == 0) {
        fridge_id = 0;
    } else {
        sem_wait(fridge_sem[1]);
        fridge_id = 1;
    }

    printf("%sBaker %d is accessing Fridge %d...%s\n",
           baker->color_code, baker->baker_id, fridge_id, "\033[0m");

    for (int i = 0; i < 3; i++) {
        if (r->required_fridge[i] == 1) {
            printf("%sBaker %d grabbed %s from Fridge %d%s\n",
                   baker->color_code,
                   baker->baker_id,
                   fridge_names[i],
                   fridge_id,
                   "\033[0m");
            usleep(100000);
        }
    }

    sem_post(fridge_sem[fridge_id]);
    return 0;
}
void printGordon() {
    const char *art[] = {
        "⠁⡼⠋⠀⣆⠀⠀⣰⣿⣫⣾⢿⣿⣿⠍⢠⠠⠀⠀⢀⠰⢾⣺⣻⣿⣿⣿⣷⡀⠀",
        "⣥⠀⠀⠀⠁⠀⠠⢻⢬⠁⣠⣾⠛⠁⠀⠀⠀⠀⠀⠀⠀⠐⠱⠏⡉⠙⣿⣿⡇⠀",
        "⢳⠀⢰⡖⠀⠀⠈⠀⣺⢰⣿⢻⣾⣶⣿⣿⣶⣶⣤⣤⣴⣾⣿⣷⣼⡆⢸⣿⣧⠀",
        "⠈⠀⠜⠈⣀⣔⣦⢨⣿⣿⣿⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣅⣼⠛⢹⠀",
        "⠀⠀⠀⠀⢋⡿⡿⣯⣭⡟⣟⣿⣿⣽⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⡘⠀",
        "⡀⠐⠀⠀⠀⣿⣯⡿⣿⣿⣿⣯⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠋⣉⢽⣿⡆⠀⠀",
        "⢳⠀⠄⠀⢀⣿⣿⣿⣿⣿⣿⣿⠙⠉⠉⠉⠛⣻⢛⣿⠛⠃⠀⠐⠛⠻⣿⡇⠀⠀",
        "⣾⠄⠀⠀⢸⣿⣿⡿⠟⠛⠁⢀⠀⢀⡄⣀⣠⣾⣿⣿⡠⣴⣎⣀⣠⣠⣿⡇⠀⠀",
        "⣧⠀⣴⣄⣽⣿⣿⣿⣶⣶⣖⣶⣬⣾⣿⣾⣿⣿⣿⣿⣽⣿⣿⣿⣿⣿⣿⡇⠀⠀",
        "⣿⣶⣈⡯⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠋⣹⢧⣿⣿⣿⣄⠙⢿⣿⣿⣿⠇⠀⠀",
        "⠹⣿⣿⣧⢌⢽⣻⢿⣯⣿⣿⣿⣿⠟⣠⡘⠿⠟⠛⠛⠟⠛⣧⡈⠻⣾⣿⠀⠀⠀",
        "⠀⠈⠉⣷⡿⣽⠶⡾⢿⣿⣿⣿⢃⣤⣿⣷⣤⣤⣄⣄⣠⣼⡿⢷⢀⣿⡏⠀⠀⠀",
        "⠀⠀⢀⣿⣷⠌⣈⣏⣝⠽⡿⣷⣾⣏⣀⣉⣉⣀⣀⣀⣠⣠⣄⡸⣾⣿⠃⠀⠀⠀",
        "⠀⣰⡿⣿⣧⡐⠄⠱⣿⣺⣽⢟⣿⣿⢿⣿⣍⠉⢀⣀⣐⣼⣯⡗⠟⡏⠀⠀⠀⠀",
        "⣰⣿⠀⣿⣿⣴⡀⠂⠘⢹⣭⡂⡚⠿⢿⣿⣿⣿⡿⢿⢿⡿⠿⢁⣴⣿⣷⣶⣦⣤"
    };

    for (int i = 0; i < 15; i++) {
        printf("%s\n", art[i]);
    }
}
void *baker_routine(void *arg) {
    BakerInfo *baker = (BakerInfo *)arg;
    
    while (baker->recipes_completed < 5) {
        baker->current_recipe = all_recipes[baker->recipes_completed];
        
        printf("%sBaker %d is starting recipe %s...\033[0m\n",
               baker->color_code, baker->baker_id, recipe_names[baker->recipes_completed]);

        getPantryIngredients(baker, baker->current_recipe);
        getFridgeIngredients(baker, baker->current_recipe);
        
        if (baker->ramsieAble && (rand() % 100 < 20)) {
            printGordon();
            printf("RAMSAY: WHERE'S THE LAMB SAUCE\?\?!? Baker %d is restarting!\n", baker->baker_id);
            baker->current_recipe = NULL;

        }
        // Order is necessary to ensure there is no deadlock
        sem_wait(bowl_sem);
        sem_wait(spoon_sem);
        sem_wait(mixer_sem);
        sem_wait(oven_sem);
        
        // Check if he has been ramsied
        if (baker->current_recipe != NULL) {
            printf("%sBaker %d is baking recipe %s!\033[0m\n",
                   baker->color_code, baker->baker_id, recipe_names[baker->recipes_completed]);
            sleep(1); // Baking time
            baker->recipes_completed++;
        } else {
            printf("%s\033[7m BAKER %d IS AN IDIOT SANDWICH! \033[0m\n",
                       baker->color_code, baker->baker_id);
        }
        
        sem_post(oven_sem);

        sem_post(mixer_sem);
        sem_post(spoon_sem);
        sem_post(bowl_sem);
        
        usleep(100000);
    }
    
    printf("%s>>> Baker %d has finished all recipes! <<<\033[0m\n",
           baker->color_code, baker->baker_id);
    
    return NULL;
}

// ----------------------------------------------------------------------------------- SETUP -------------------------------------------------------------------------------
int main() {
    srand(time(NULL));
    
    printf("Enter the number of bakers: \n");
    int num_bakers;
    if (scanf("%d", &num_bakers) != 1) {
        fprintf(stderr, "Error: Invalid input.\n");
        return 1;
    }

    // Initialize resources
    initializeRecipes();
    initializeSemaphores();

    printf("Starting Bake Off with %d bakers\n", num_bakers);
    printf("--------------------------------------------------\n");
    // Arrays to hold thread IDs and baker data
    pthread_t threads[num_bakers];
    BakerInfo *bakers[num_bakers];

    // Choose a victim
    int victim_id = rand() % num_bakers;
    
    for (int i = 0; i < num_bakers; i++) {
        bakers[i] = malloc(sizeof(BakerInfo));
        bakers[i]->baker_id = i;
        bakers[i]->recipes_completed = 0;
        bakers[i]->current_recipe = NULL;
        
        // Dynamic color assignment
        int color_val = 20 + (i * 210 / num_bakers);
        bakers[i]->color_code = malloc(20 * sizeof(char));
        sprintf(bakers[i]->color_code, "\033[38;5;%dm", color_val);

        if (i == victim_id) {
            bakers[i]->ramsieAble = 1;
            printf("Note: Baker %d is being watched by Chef Ramsay...\n", i);
        } else {
            bakers[i]->ramsieAble = 0;
        }

        if (pthread_create(&threads[i], NULL, baker_routine, bakers[i]) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }
    
    
    // CLEANUP
    // Wait for all baker threads to finish
    for (int i = 0; i < num_bakers; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("--------------------------------------------------\n");
    printf("All bakers have finished baking!\n");

    for (int i = 0; i < num_bakers; i++) {
        free(bakers[i]);
    }

    sem_close(mixer_sem);
    sem_close(pantry_sem);
    sem_close(fridge_sem[0]);
    sem_close(fridge_sem[1]);
    sem_close(bowl_sem);
    sem_close(spoon_sem);
    sem_close(oven_sem);

    sem_unlink("/mixer");
    sem_unlink("/pantry");
    sem_unlink("/fridge0");
    sem_unlink("/fridge1");
    sem_unlink("/bowl");
    sem_unlink("/spoon");
    sem_unlink("/oven");

    return 0;
}
