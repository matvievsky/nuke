# nuke
My solution to the Disk Covering Problem with the most points

The roblem:
Determine the coordinates of the most effective counter-strike (with the greatest damage to the enemy) for a single warhead. The two-dimensional coordinates for each object are known.
Coordinates are pairs of numbers, limited to positive integers in the range [0..99], they can be repeated.

```
Makefile
make all
make clean
```
The program accepts a file-list of coordinates and the radius of destruction of the warhead as input:
```
./nuke coords.txt 30
```
The program should display the location of the strike and the number of potentially hit targets:
```
Optimal coordinates are {40, 36} with 597 target(s) to destroy.
```
You can generate a map of a given number of points and a range of values (by default it's 10 and 100 respectively):
```
./map_generator 2000 100
Map of 2000 target(s) with size = 100 enerated successfully
```

You can run unit tests for verification. To do this, you need to remove the comment on lines [66..68]
