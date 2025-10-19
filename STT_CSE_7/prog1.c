#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
   Student Record Management System
   --------------------------------
   Features:
   1. Add new student
   2. Display all students
   3. Search student by roll number
   4. Update student record
   5. Delete student
   6. Sort students by marks
   7. Exit

   Uses:
   - loops (while, for)
   - conditionals (if/else, switch)
   - multiple variables with reassignments
*/

#define MAX_STUDENTS 100

struct Student {
    int roll;
    char name[50];
    float marks;
};

struct Student students[MAX_STUDENTS];
int count = 0; // number of students currently stored

/* Function prototypes */
void addStudent();
void displayStudents();
void searchStudent();
void updateStudent();
void deleteStudent();
void sortStudents();
void printMenu();
void pause();

/* Main function */
int main() {
    int choice;

    /* Main menu loop */
    while (1) {
        printMenu();
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Clearing buffer...\n");
            while (getchar() != '\n'); // clear input buffer
            continue;
        }

        switch (choice) {
            case 1: addStudent(); break;
            case 2: displayStudents(); break;
            case 3: searchStudent(); break;
            case 4: updateStudent(); break;
            case 5: deleteStudent(); break;
            case 6: sortStudents(); break;
            case 7: printf("Exiting program...\n"); exit(0);
            default: printf("Invalid choice. Try again.\n");
        }
        pause();
    }
    return 0;
}

/* Print menu options */
void printMenu() {
    printf("\n==============================\n");
    printf(" Student Record Management \n");
    printf("==============================\n");
    printf("1. Add Student\n");
    printf("2. Display Students\n");
    printf("3. Search Student by Roll\n");
    printf("4. Update Student\n");
    printf("5. Delete Student\n");
    printf("6. Sort Students by Marks\n");
    printf("7. Exit\n");
    printf("==============================\n");
}

/* Add a student */
void addStudent() {
    if (count >= MAX_STUDENTS) {
        printf("Maximum student limit reached!\n");
        return;
    }
    struct Student s;
    printf("Enter roll number: ");
    scanf("%d", &s.roll);
    printf("Enter name: ");
    scanf("%s", s.name);
    printf("Enter marks: ");
    scanf("%f", &s.marks);

    students[count] = s; // copy to array
    count++;
    printf("Student added successfully!\n");
}

/* Display all students */
void displayStudents() {
    if (count == 0) {
        printf("No students to display.\n");
        return;
    }
    printf("\nRoll\tName\tMarks\n");
    printf("-------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%d\t%s\t%.2f\n", students[i].roll, students[i].name, students[i].marks);
    }
}

/* Search student by roll number */
void searchStudent() {
    if (count == 0) {
        printf("No students to search.\n");
        return;
    }
    int roll;
    printf("Enter roll number to search: ");
    scanf("%d", &roll);

    for (int i = 0; i < count; i++) {
        if (students[i].roll == roll) {
            printf("Student found: %d %s %.2f\n",
                   students[i].roll, students[i].name, students[i].marks);
            return;
        }
    }
    printf("Student not found.\n");
}

/* Update student record */
void updateStudent() {
    if (count == 0) {
        printf("No students to update.\n");
        return;
    }
    int roll;
    printf("Enter roll number to update: ");
    scanf("%d", &roll);

    for (int i = 0; i < count; i++) {
        if (students[i].roll == roll) {
            printf("Enter new name: ");
            scanf("%s", students[i].name);
            printf("Enter new marks: ");
            scanf("%f", &students[i].marks);
            printf("Record updated successfully.\n");
            return;
        }
    }
    printf("Student not found.\n");
}

/* Delete a student record */
void deleteStudent() {
    if (count == 0) {
        printf("No students to delete.\n");
        return;
    }
    int roll;
    printf("Enter roll number to delete: ");
    scanf("%d", &roll);

    for (int i = 0; i < count; i++) {
        if (students[i].roll == roll) {
            // shift remaining
            for (int j = i; j < count - 1; j++) {
                students[j] = students[j + 1];
            }
            count--;
            printf("Record deleted successfully.\n");
            return;
        }
    }
    printf("Student not found.\n");
}

/* Sort students by marks descending */
void sortStudents() {
    if (count == 0) {
        printf("No students to sort.\n");
        return;
    }
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (students[j].marks < students[j + 1].marks) {
                struct Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
    printf("Students sorted by marks (descending).\n");
}

/* Pause to let user read output */
void pause() {
    printf("Press Enter to continue...");
    while (getchar() != '\n'); // clear buffer
    getchar(); // wait for Enter
}

/* 
Extra notes:
------------
This program uses:
- Structs for student records
- Arrays to store up to MAX_STUDENTS
- Loops for menus and operations
- Conditionals for checking input and search results
- Multiple variables with reassignments (marks, roll, count)
End of file.
*/
