#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <assert.h>

static const unsigned int GRID_SIZE = 100;
typedef struct {
    int x, y;
} Target;
typedef struct {
    Target target;
    unsigned int number_of_targets;
} Nuke;
typedef struct List {
    Target target;
    struct List *next;
} List;

#define ERROR_HANDLER(x, ern) ErrorHandler(x, ern)
void ErrorHandler(const char *error_text, const int ern);
void AddTargetToList(const Target target, List *list);
void DestroyList(List *list);
unsigned int FloatComparator(const float *f1, const float *f2);
unsigned int GetTargets(FILE *stream, List *target_list);
float GetDistance(const Target *target1, const Target *target2);
Target GetCenterCoordinates(const float *x, const float *y,
                            const Target *p1, const Target *p2, const float *distance);
Nuke GetOptimalCoordinates(const List *list, const float radius);
void test(const Nuke *optimal, const List *list, const float radius);

int main(const int argc, const char* argv[]) {
    if (argc != 3) {
        ERROR_HANDLER("Wrong number of arguments", 1);
    }
    List target_list;
    unsigned int targets_amount = 0;
    FILE* stream = fopen(argv[1], "rt");
    if (stream) {
        targets_amount = GetTargets(stream, &target_list);
        fclose(stream);
        if (!targets_amount) {
            DestroyList(&target_list);
            ERROR_HANDLER("Nothing to nuke or coordinates list is broken", 1);
        }
    } else {
        DestroyList(&target_list);
        ERROR_HANDLER("Unable to open coordinates file", 1);
    }
    int radius = atoi(argv[2]);
    if (radius <= 0) {
        DestroyList(&target_list);
        ERROR_HANDLER("Wrong radius of destruction", 1);
    }
    Nuke point_of_impact;
    if (radius < GRID_SIZE * 0.5 * sqrtf(2.0)) {
        point_of_impact = GetOptimalCoordinates(&target_list, (float)radius);
    } else {
        point_of_impact = (Nuke){
            {GRID_SIZE / 2 + GRID_SIZE % 2, GRID_SIZE / 2 + GRID_SIZE % 2},
            targets_amount
        };
    }
    printf("Optimal coordinates are {%d, %d} with %d target(s) to destroy.\n",
    point_of_impact.target.x, point_of_impact.target.y, point_of_impact.number_of_targets);
    // printf("Damage efficency is %.2f%%\n",  (float)point_of_impact.number_of_targets /
    //                                         (float)targets_amount * 100);
    // test(&point_of_impact, &target_list, (float)radius);
    DestroyList(&target_list);
    exit(EXIT_SUCCESS);
}

void ErrorHandler(const char *error_text, const int ern) {
    errno = ern;
    perror(error_text);
    exit(EXIT_FAILURE);
}
void AddTargetToList(const Target target, List *list) {
    List *temp = list->next;
    list->next = (List*)malloc(sizeof(List));
    list->next->next = temp;
    list->next->target = target;
}
void DestroyList(List *list) {
    while (list->next) {
        list->next->target = (Target){0, 0};
        List *temp = list->next->next;
        free(list->next);
        list->next = temp;
    }
}
unsigned int FloatComparator(const float *f1, const float *f2) {
    return fabsf(*f1 - *f2) < 1e-6;
}
unsigned int GetTargets(FILE *stream, List *list) {
    unsigned int targets_amount = 0, x, y;
    while (fscanf(stream,"%u,%u", &x, &y) != EOF) {
        if (x < GRID_SIZE && y < GRID_SIZE) {
            AddTargetToList((Target){x, y}, list);
            ++targets_amount;
        } else {
            return 0;
        }
    }
    return targets_amount;
}
float GetDistance(const Target *target1, const Target *target2) {
    return sqrtf(   (target1->x - target2->x) * (target1->x - target2->x) +
                    (target1->y - target2->y) * (target1->y - target2->y)   );
}
Target GetCenterCoordinates(const float *x, const float *y,
                            const Target *p1, const Target *p2, const float *distance) {
    int u_x = *x, u_y = *y;
    if (*x == u_x && *y == u_y) {
        return (Target){u_x, u_y};
    }
    float f_x = floorf(*x), f_y = floorf(*y), c_x = ceilf(*x), c_y = ceilf(*y);
    Target rounded[4] = {
        {f_x, f_y},
        {f_x, c_y},
        {c_x, f_y},
        {c_x, c_y}
    };
    float lengths[4] = {
        fabsf(  (p2->y - p1->y) * f_x - (p2->x - p1->x) * f_y +
                p2->x * p1->y - p2->y * p1->x                   ) / *distance,
        fabsf(  (p2->y - p1->y) * f_x - (p2->x - p1->x) * c_y +
                p2->x * p1->y - p2->y * p1->x                   ) / *distance,
        fabsf(  (p2->y - p1->y) * c_x - (p2->x - p1->x) * f_y +
                p2->x * p1->y - p2->y * p1->x                   ) / *distance,
        fabsf(  (p2->y - p1->y) * c_x - (p2->x - p1->x) * c_y +
                p2->x * p1->y - p2->y * p1->x                   ) / *distance
    };
    unsigned int max = 0, prev = 0;
    for (unsigned int i = 1; i < 4; ++i) {
        if (lengths[i] > lengths[max]) {
            prev = max;
            max = i;
        }
    }
    return rounded[prev];
}
Nuke GetOptimalCoordinates(const List *list, const float radius) {
    Nuke optimal = {list->next->target, 1}, current;
    for (List *p1 = list->next; p1; p1 = p1->next) {
        for (List *p2 = p1->next; p2; p2 = p2->next) {
            const float distance = GetDistance(&p1->target, &p2->target);
            const float diameter = 2.0 * radius;
            if (distance < diameter || FloatComparator(&distance, &diameter)) {
                const float height = sqrtf((radius * radius) - 0.25 * distance * distance);
                float x, y;
                for (int sign = 1; sign >= -1; sign -= 2) {
                    if (distance) {
                        x =  0.5 * (p1->target.x + p2->target.x) + 
                                    sign * (p1->target.y - p2->target.y) * height / distance;
                        y =  0.5 * (p1->target.y + p2->target.y) +
                                    sign * (p2->target.x - p1->target.x) * height / distance;
                        if (x < 0.0 || y < 0.0) {
                            continue;
                        }
                        current.target = GetCenterCoordinates(  &x, &y, &p1->target,
                                                                &p2->target, &distance  );
                    } else {
                        current.target = p1->target;
                        sign = -2;
                    }
                    current.number_of_targets = 0;
                    float deviation;
                    for (List *p3 = list->next; p3; p3 = p3->next) {
                        deviation = GetDistance(&current.target, &p3->target);
                        current.number_of_targets +=    (deviation < radius) ||
                                                        FloatComparator(&deviation, &radius);
                    }
                    if (optimal.number_of_targets < current.number_of_targets) {
                        optimal = current;
                    }
                }
            }
        }
    }
    return optimal;
}

void test(const Nuke *optimal, const List *list, const float radius) {
    //  unit test will try to search around and catch mistake if aproximation was incorrect
    const unsigned int n = optimal->number_of_targets;
    //  ###
    //  #*#
    //  ###
    while (1) {
        int x = optimal->target.x;
        int y = optimal->target.y;
        float deviation = 0;
        unsigned int counter = 0;
        Target ceneter = {x, y};
        for (List *p1 = list->next; p1; p1 = p1->next) {
            deviation = GetDistance(&ceneter, &p1->target);
            counter += (deviation < radius) || FloatComparator(&deviation, &radius);
        }
        printf("Checking {%d, %d}, n = %d\n", x, y, counter);
        assert(counter == n);
        break;
    }
    //  #*#
    //  ###
    //  ###
    while (1) {
        int x = optimal->target.x;
        int y = optimal->target.y;
        float deviation = 0;
        unsigned int counter = 0;
        if (++y >= (int)GRID_SIZE) {
            break;
        }
        Target ceneter = {x, y};
        for (List *p1 = list->next; p1; p1 = p1->next) {
            deviation = GetDistance(&ceneter, &p1->target);
            counter += (deviation < radius) || FloatComparator(&deviation, &radius);
        }
        printf("Checking {%d, %d}, n = %d\n", x, y, counter);
        assert(counter <= n);
        break;
    }
    //  ##*
    //  ###
    //  ###
    while (1) {
        int x = optimal->target.x;
        int y = optimal->target.y;
        float deviation = 0;
        unsigned int counter = 0;
        if (++y >= (int)GRID_SIZE || ++x >= (int)GRID_SIZE) {
            break;
        }
        Target ceneter = {x + 1, y + 1};
        for (List *p1 = list->next; p1; p1 = p1->next) {
            deviation = GetDistance(&ceneter, &p1->target);
            counter += (deviation < radius) || FloatComparator(&deviation, &radius);
        }
        printf("Checking {%d, %d}, n = %d\n", x, y, counter);
        assert(counter <= n);
        break;
    }
    //  ###
    //  ##*
    //  ###
    while (1) {
        int x = optimal->target.x;
        int y = optimal->target.y;
        float deviation = 0;
        unsigned int counter = 0;
        if (++x >= (int)GRID_SIZE) {
            break;
        }
        Target ceneter = {x, y};
        for (List *p1 = list->next; p1; p1 = p1->next) {
            deviation = GetDistance(&ceneter, &p1->target);
            counter += (deviation < radius) || FloatComparator(&deviation, &radius);
        }
        printf("Checking {%d, %d}, n = %d\n", x, y, counter);
        assert(counter <= n);
        break;
    }
    //  ###
    //  ###
    //  ##*
    while (1) {
        int x = optimal->target.x;
        int y = optimal->target.y;
        float deviation = 0;
        unsigned int counter = 0;
        if (++x >= (int)GRID_SIZE || --y < 0) {
            break;
        }
        Target ceneter = {x, y};
        for (List *p1 = list->next; p1; p1 = p1->next) {
            deviation = GetDistance(&ceneter, &p1->target);
            counter += (deviation < radius) || FloatComparator(&deviation, &radius);
        }
        printf("Checking {%d, %d}, n = %d\n", x, y, counter);
        assert(counter <= n);
        break;
    }
    //  ###
    //  ###
    //  #*#
    while (1) {
        int x = optimal->target.x;
        int y = optimal->target.y;
        float deviation = 0;
        unsigned int counter = 0;
        if (--y < 0) {
            break;
        }
        Target ceneter = {x, y};
        for (List *p1 = list->next; p1; p1 = p1->next) {
            deviation = GetDistance(&ceneter, &p1->target);
            counter += (deviation < radius) || FloatComparator(&deviation, &radius);
        }
        printf("Checking {%d, %d}, n = %d\n", x, y, counter);
        assert(counter <= n);
        break;
    }
    //  ###
    //  ###
    //  *##
    while (1) {
        int x = optimal->target.x;
        int y = optimal->target.y;
        float deviation = 0;
        unsigned int counter = 0;
        if (--y < 0 || --x < 0) {
            break;
        }
        Target ceneter = {x + 1, y + 1};
        for (List *p1 = list->next; p1; p1 = p1->next) {
            deviation = GetDistance(&ceneter, &p1->target);
            counter += (deviation < radius) || FloatComparator(&deviation, &radius);
        }
        printf("Checking {%d, %d}, n = %d\n", x, y, counter);
        assert(counter <= n);
        break;
    }
    //  ###
    //  *##
    //  ###
    while (1) {
        int x = optimal->target.x;
        int y = optimal->target.y;
        float deviation = 0;
        unsigned int counter = 0;
        if (--x < 0) {
            break;
        }
        Target ceneter = {x, y};
        for (List *p1 = list->next; p1; p1 = p1->next) {
            deviation = GetDistance(&ceneter, &p1->target);
            counter += (deviation < radius) || FloatComparator(&deviation, &radius);
        }
        printf("Checking {%d, %d}, n = %d\n", x, y, counter);
        assert(counter <= n);
        break;
    }
    //  *##
    //  ###
    //  ###
    while (1) {
        int x = optimal->target.x;
        int y = optimal->target.y;
        float deviation = 0;
        unsigned int counter = 0;
        if (--x < 0 || ++y >= (int)GRID_SIZE) {
            break;
        }
        Target ceneter = {x, y};
        for (List *p1 = list->next; p1; p1 = p1->next) {
            deviation = GetDistance(&ceneter, &p1->target);
            counter += (deviation < radius) || FloatComparator(&deviation, &radius);
        }
        printf("Checking {%d, %d}, n = %d\n", x, y, counter);
        assert(counter <= n);
        break;
    }
}
