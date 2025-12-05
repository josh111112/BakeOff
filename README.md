Hell's Kitchen: A Concurrency Simulation
Hell's Kitchen is a multi-threaded C application that simulates a high-pressure baking environment. It utilizes POSIX threads to represent bakers and POSIX semaphores to represent limited kitchen resources. The simulation manages resource contention, deadlock avoidance, and random external interrupts (simulated as the wrath of Chef Ramsay).

System Requirements
Operating System: Linux or macOS (POSIX compliant).
Compiler: GCC or Clang with support for pthread.
Note: This application utilizes headers (<semaphore.h>, <unistd.h>, <pthread.h>) that are not natively supported on Windows.
The Simulation Scenario
The program initializes a user-defined number of bakers. Each baker is a separate thread attempting to complete a specific queue of 5 recipes in order:
Cookies
Pancakes
Homemade Pizza
Soft Pretzels
Cinnamon Rolls
To bake these items, bakers must acquire specific ingredients from the pantry and fridge, and then acquire a specific set of kitchen tools to perform the baking.

The Kitchen Resources (Semaphores)
The kitchen is resource-constrained. If a baker needs a tool that is currently in use, they must wait (block) until it becomes available.

Resource    Count    Type    Description
Pantry    1    Mutex    Only one person fits in the pantry at a time.
Fridge    2    Counting    Two separate fridge units available.
Bowl    3    Counting    Only 3 mixing bowls available.
Spoon    5    Counting    5 mixing spoons available.
Mixer    2    Counting    2 electric mixers available.
Oven    1    Mutex    The ultimate bottleneck. Only one oven.
Implementation Details
This project demonstrates several key operating system concepts:

1. POSIX Threads
Each baker runs the baker_routine function as a unique thread. They operate asynchronously, competing for resources in real-time.

2. Resource Protection & Synchronization
Semaphores are used to control access to shared resources.
The Pantry: Uses a binary semaphore (mutex) to ensure mutual exclusion.
The Fridge: Implemented as two separate semaphores (fridge0, fridge1). The baker attempts to access Fridge 0; if occupied, they wait for Fridge 1.
3. Deadlock Avoidance
The simulation implements a strict resource acquisition hierarchy to prevent circular wait conditions (deadlocks). When the actual baking phase begins, resources are acquired in this specific order:
sem_wait(bowl)
sem_wait(spoon)
sem_wait(mixer)
sem_wait(oven)
Resources are released in reverse order. This ensures that a baker holding the oven will not get stuck waiting for a bowl held by someone waiting for the oven.

4. The "Ramsay Factor" (Random Interrupts)
At the start of execution, one thread is randomly designated as the "Victim."
Every time the victim attempts to bake, there is a 20% chance rand() triggers a specialized event.
The Event: ASCII art of Chef Ramsay appears, the baker is yelled at ("WHERE'S THE LAMB SAUCE??"), and the baker is forced to restart the current recipe immediately, discarding their progress.
If the baker fails the check, they are labeled an "Idiot Sandwich" in the console output.
5. User Interface
ANSI Color Codes: Each baker is dynamically assigned a unique color based on their ID to make the console output readable and to visually track thread interleaving.
Visual Feedback: The console prints exactly when threads are waiting for resources, grabbing ingredients, or baking.
How to Compile and Run
Clone the repository or save the file as hells_kitchen.c. Open your terminal and run the following commands.

1. Compilation You must link the pthread library.

Bash
gcc -o kitchen hells_kitchen.c -pthread
2. Execution Run the executable.

Bash
./kitchen
3. Interaction Once the program starts, it will prompt you for input:

Plaintext
Enter the number of bakers:
Type an integer (e.g., 5 or 10) and press Enter. The simulation will immediately begin logging thread activity to the console.

Code Map
If you wish to modify the simulation parameters, refer to these sections in hells_kitchen.c:
Modify Ingredient Lists: Check initializeRecipes() to change what items are required from the pantry or fridge.
Adjust Resource Counts: Look at initializeSemaphores(). For example, changing mixer_sem from 2 to 1 will drastically increase contention.
Change Recipes: The RecipeName enum and all_recipes array control the bake list.
Tweak Ramsay's Anger: In baker_routine, modify the line rand() % 100 < 20 to change the probability of the victim being interrupted.
Clean Up
The program automatically handles memory cleanup. It joins all threads, frees allocated BakerInfo and Recipe structs, closes semaphores, and unlinks them from the system to ensure a clean exit.
