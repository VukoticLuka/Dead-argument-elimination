#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a struct Person
typedef struct {
    char name[50];
    int age;
} Person;

// Function that modifies a Person and takes an unused argument
void modify_person(Person *p, int *unused_value) {
    // 'unused_value' is a dead argument
    strcpy(p->name, "John Doe");
    p->age = 30;
}

int main() {
    Person *p = (Person *)malloc(sizeof(Person));
    int unused_variable = 100;
    
    modify_person(p, &unused_variable); // 'unused_variable' is passed but never used
    
    printf("Name: %s, Age: %d\n", p->name, p->age); // Output: Name: John Doe, Age: 30
    
    free(p);
    return 0;
}
