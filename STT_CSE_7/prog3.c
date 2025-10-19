#include <stdio.h>
#include <stdlib.h>

/*
   Matrix Operations Program
   -------------------------
   Features:
   1. Add two matrices
   2. Subtract two matrices
   3. Multiply two matrices
   4. Transpose a matrix
   5. Check if matrix is symmetric
   6. Exit

   Demonstrates:
   - Loops (for, while)
   - Conditionals (if/else, switch)
   - Multiple variables with reassignments
*/

#define MAX 20

/* Function prototypes */
void inputMatrix(int rows, int cols, int m[MAX][MAX]);
void printMatrix(int rows, int cols, int m[MAX][MAX]);
void addMatrices(int r, int c, int a[MAX][MAX], int b[MAX][MAX], int res[MAX][MAX]);
void subtractMatrices(int r, int c, int a[MAX][MAX], int b[MAX][MAX], int res[MAX][MAX]);
void multiplyMatrices(int r1, int c1, int a[MAX][MAX], int r2, int c2, int b[MAX][MAX], int res[MAX][MAX]);
void transposeMatrix(int r, int c, int a[MAX][MAX], int res[MAX][MAX]);
int isSymmetric(int r, int c, int a[MAX][MAX]);
void pauseScreen();

int main() {
    int choice;
    int r1, c1, r2, c2;
    int A[MAX][MAX], B[MAX][MAX], result[MAX][MAX];

    while (1) {
        printf("\n===== Matrix Operations Menu =====\n");
        printf("1. Add two matrices\n");
        printf("2. Subtract two matrices\n");
        printf("3. Multiply two matrices\n");
        printf("4. Transpose a matrix\n");
        printf("5. Check if matrix is symmetric\n");
        printf("6. Exit\n");
        printf("==================================\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Clearing buffer.\n");
            while (getchar() != '\n');
            continue;
        }

        if (choice == 6) {
            printf("Exiting program...\n");
            break;
        }

        switch (choice) {
            case 1: /* Add matrices */
                printf("Enter rows and columns: ");
                scanf("%d %d", &r1, &c1);
                printf("Enter Matrix A:\n");
                inputMatrix(r1, c1, A);
                printf("Enter Matrix B:\n");
                inputMatrix(r1, c1, B);
                addMatrices(r1, c1, A, B, result);
                printf("Resultant Matrix (A+B):\n");
                printMatrix(r1, c1, result);
                break;

            case 2: /* Subtract matrices */
                printf("Enter rows and columns: ");
                scanf("%d %d", &r1, &c1);
                printf("Enter Matrix A:\n");
                inputMatrix(r1, c1, A);
                printf("Enter Matrix B:\n");
                inputMatrix(r1, c1, B);
                subtractMatrices(r1, c1, A, B, result);
                printf("Resultant Matrix (A-B):\n");
                printMatrix(r1, c1, result);
                break;

            case 3: /* Multiply matrices */
                printf("Enter rows and columns of Matrix A: ");
                scanf("%d %d", &r1, &c1);
                printf("Enter Matrix A:\n");
                inputMatrix(r1, c1, A);
                printf("Enter rows and columns of Matrix B: ");
                scanf("%d %d", &r2, &c2);
                printf("Enter Matrix B:\n");
                inputMatrix(r2, c2, B);
                if (c1 != r2) {
                    printf("Matrix multiplication not possible. Columns of A != Rows of B.\n");
                } else {
                    multiplyMatrices(r1, c1, A, r2, c2, B, result);
                    printf("Resultant Matrix (A*B):\n");
                    printMatrix(r1, c2, result);
                }
                break;

            case 4: /* Transpose */
                printf("Enter rows and columns: ");
                scanf("%d %d", &r1, &c1);
                printf("Enter Matrix:\n");
                inputMatrix(r1, c1, A);
                transposeMatrix(r1, c1, A, result);
                printf("Transpose Matrix:\n");
                printMatrix(c1, r1, result);
                break;

            case 5: /* Symmetric check */
                printf("Enter rows and columns: ");
                scanf("%d %d", &r1, &c1);
                printf("Enter Matrix:\n");
                inputMatrix(r1, c1, A);
                if (isSymmetric(r1, c1, A)) {
                    printf("Matrix is symmetric.\n");
                } else {
                    printf("Matrix is not symmetric.\n");
                }
                break;

            default:
                printf("Invalid choice.\n");
        }

        pauseScreen();
    }

    return 0;
}

/* Input matrix */
void inputMatrix(int rows, int cols, int m[MAX][MAX]) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("Enter element [%d][%d]: ", i, j);
            scanf("%d", &m[i][j]);
        }
    }
}

/* Print matrix */
void printMatrix(int rows, int cols, int m[MAX][MAX]) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%4d ", m[i][j]);
        }
        printf("\n");
    }
}

/* Add two matrices */
void addMatrices(int r, int c, int a[MAX][MAX], int b[MAX][MAX], int res[MAX][MAX]) {
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            res[i][j] = a[i][j] + b[i][j];
        }
    }
}

/* Subtract two matrices */
void subtractMatrices(int r, int c, int a[MAX][MAX], int b[MAX][MAX], int res[MAX][MAX]) {
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            res[i][j] = a[i][j] - b[i][j];
        }
    }
}

/* Multiply two matrices */
void multiplyMatrices(int r1, int c1, int a[MAX][MAX], int r2, int c2, int b[MAX][MAX], int res[MAX][MAX]) {
    for (int i = 0; i < r1; i++) {
        for (int j = 0; j < c2; j++) {
            res[i][j] = 0; // reassigning
            for (int k = 0; k < c1; k++) {
                res[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

/* Transpose a matrix */
void transposeMatrix(int r, int c, int a[MAX][MAX], int res[MAX][MAX]) {
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            res[j][i] = a[i][j];
        }
    }
}

/* Check if matrix is symmetric */
int isSymmetric(int r, int c, int a[MAX][MAX]) {
    if (r != c) return 0;
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            if (a[i][j] != a[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

/* Pause to continue */
void pauseScreen() {
    printf("Press Enter to continue...");
    while (getchar() != '\n');
    getchar();
}


