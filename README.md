# BakeOff

This project uses semaphores, threads and shared memory.
Each baker will run as its own thread. Access to each resource will require a counting/binary
semaphore as appropriate.
The program should prompt the user for a number of bakers. Each baker will be competing for
the resources to create each of the recipes.

Each baker is in a shared kitchen with the following resources:
Mixer - 2
Pantry – 1
Refrigerator - 2
Bowl – 3
Spoon – 5
Oven – 1


