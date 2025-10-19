#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
   Simple Library Management System
   --------------------------------
   Features:
   1. Add new book
   2. Display all books
   3. Search book by title
   4. Issue a book
   5. Return a book
   6. Remove a book
   7. Exit

   Demonstrates:
   - Conditionals (if/else)
   - Loops (while, for)
   - Multiple variables with reassignments
*/

#define MAX_BOOKS 100
#define TITLE_LEN 50
#define AUTHOR_LEN 50

typedef struct {
    int id;
    char title[TITLE_LEN];
    char author[AUTHOR_LEN];
    int totalCopies;
    int availableCopies;
} Book;

Book library[MAX_BOOKS];
int bookCount = 0;

/* Function prototypes */
void addBook();
void displayBooks();
void searchBook();
void issueBook();
void returnBook();
void removeBook();
void pauseScreen();

int main() {
    int choice;

    while (1) {
        printf("\n===== Library Management Menu =====\n");
        printf("1. Add new book\n");
        printf("2. Display all books\n");
        printf("3. Search book by title\n");
        printf("4. Issue a book\n");
        printf("5. Return a book\n");
        printf("6. Remove a book\n");
        printf("7. Exit\n");
        printf("===================================\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Clearing buffer.\n");
            while (getchar() != '\n');
            continue;
        }

        if (choice == 7) {
            printf("Exiting program...\n");
            break;
        }

        switch (choice) {
            case 1: addBook(); break;
            case 2: displayBooks(); break;
            case 3: searchBook(); break;
            case 4: issueBook(); break;
            case 5: returnBook(); break;
            case 6: removeBook(); break;
            default: printf("Invalid choice.\n");
        }

        pauseScreen();
    }

    return 0;
}

/* Add a new book */
void addBook() {
    if (bookCount >= MAX_BOOKS) {
        printf("Library full. Cannot add more books.\n");
        return;
    }
    Book b;
    b.id = bookCount + 1;
    printf("Enter book title: ");
    while (getchar() != '\n'); // clear buffer
    fgets(b.title, TITLE_LEN, stdin);
    b.title[strcspn(b.title, "\n")] = 0; // remove newline
    printf("Enter author: ");
    fgets(b.author, AUTHOR_LEN, stdin);
    b.author[strcspn(b.author, "\n")] = 0;
    printf("Enter total copies: ");
    scanf("%d", &b.totalCopies);
    if (b.totalCopies < 0) b.totalCopies = 0;
    b.availableCopies = b.totalCopies;

    library[bookCount] = b; // copy struct
    bookCount++;
    printf("Book added successfully with ID %d.\n", b.id);
}

/* Display all books */
void displayBooks() {
    if (bookCount == 0) {
        printf("No books in library.\n");
        return;
    }
    printf("\n%-5s %-30s %-20s %-10s %-10s\n", "ID", "Title", "Author", "Total", "Available");
    printf("----------------------------------------------------------------------------\n");
    for (int i = 0; i < bookCount; i++) {
        printf("%-5d %-30s %-20s %-10d %-10d\n",
               library[i].id,
               library[i].title,
               library[i].author,
               library[i].totalCopies,
               library[i].availableCopies);
    }
}

/* Search book by title */
void searchBook() {
    char searchTitle[TITLE_LEN];
    printf("Enter title to search: ");
    while (getchar() != '\n');
    fgets(searchTitle, TITLE_LEN, stdin);
    searchTitle[strcspn(searchTitle, "\n")] = 0;

    int found = 0;
    for (int i = 0; i < bookCount; i++) {
        if (strstr(library[i].title, searchTitle) != NULL) {
            if (!found) {
                printf("\n%-5s %-30s %-20s %-10s %-10s\n", "ID", "Title", "Author", "Total", "Available");
                printf("----------------------------------------------------------------------------\n");
            }
            found = 1;
            printf("%-5d %-30s %-20s %-10d %-10d\n",
                   library[i].id,
                   library[i].title,
                   library[i].author,
                   library[i].totalCopies,
                   library[i].availableCopies);
        }
    }
    if (!found) {
        printf("No book found with title containing \"%s\".\n", searchTitle);
    }
}

/* Issue a book */
void issueBook() {
    int id;
    printf("Enter book ID to issue: ");
    scanf("%d", &id);
    if (id <= 0 || id > bookCount) {
        printf("Invalid ID.\n");
        return;
    }
    Book *b = &library[id - 1];
    if (b->availableCopies > 0) {
        b->availableCopies--; // reassigning variable
        printf("Book \"%s\" issued successfully. Remaining copies: %d\n", b->title, b->availableCopies);
    } else {
        printf("No available copies to issue.\n");
    }
}

/* Return a book */
void returnBook() {
    int id;
    printf("Enter book ID to return: ");
    scanf("%d", &id);
    if (id <= 0 || id > bookCount) {
        printf("Invalid ID.\n");
        return;
    }
    Book *b = &library[id - 1];
    if (b->availableCopies < b->totalCopies) {
        b->availableCopies++; // reassigning variable
        printf("Book \"%s\" returned successfully. Available copies: %d\n", b->title, b->availableCopies);
    } else {
        printf("All copies are already in library.\n");
    }
}

/* Remove a book */
void removeBook() {
    int id;
    printf("Enter book ID to remove: ");
    scanf("%d", &id);
    if (id <= 0 || id > bookCount) {
        printf("Invalid ID.\n");
        return;
    }
    // Shift remaining books one step back
    for (int i = id - 1; i < bookCount - 1; i++) {
        library[i] = library[i + 1];
        library[i].id = i + 1; // reassign ID
    }
    bookCount--;
    printf("Book removed successfully.\n");
}

/* Pause to continue */
void pauseScreen() {
    printf("Press Enter to continue...");
    while (getchar() != '\n');
    getchar();
}

/*
This program uses:
- Loops: while(1) for menu, for() loops to list/search/shift books
- Conditionals: if/else for ID validation, availability, searching, etc.
- Multiple variables: bookCount, availableCopies, IDs updated/reassigned
- ~300 LOC including comments
End of file.
*/
