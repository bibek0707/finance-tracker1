// finance_tracker.c
// Secure Finance Tracker with Register, Login, and Income/Expense Management

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 32
#define MAX_LINE_LEN 256
#define MAX_CATEGORY_LEN 50
#define MAX_DATE_LEN 30
#define USER_FILE "users.dat"
#define RECORD_FILE "record.bin"
#define INCOME_FILE "myincome.bin"
#define EXPENSE_FILE "myexpense.bin"

struct Node {
    char date[MAX_DATE_LEN];
    double amount;
    char category[MAX_CATEGORY_LEN];
    struct Node *next;
};

struct Record {
    double income;
    double expense;
};

// Global variables
struct Node *incomeList = NULL, *expenseList = NULL;
double currentIncome = 0, currentExpense = 0;

// Function Prototypes
void clearStdin();
char* secureInput(char* buffer, size_t size);
void hashPassword(const char* password, char* hashed);
void registerUser();
int loginUser();
void showMainMenu();
void showFinanceMenu();
void addIncome();
void addExpense();
void displayRecords(struct Node *list, const char *type);
void saveIncome();
void saveExpense();
void loadIncome();
void loadExpense();
void saveRecord();
void loadRecord();
void freeList(struct Node *list);

// Simple hash function
void hashPassword(const char* password, char* hashed) {
    unsigned int hash = 5381;
    for (int i = 0; password[i]; i++)
        hash = ((hash << 5) + hash) + password[i];
    sprintf(hashed, "%u", hash);
}

char* secureInput(char* buffer, size_t size) {
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        return buffer;
    } else {
        fprintf(stderr, "[ERROR] Failed to read input.\n");
        exit(EXIT_FAILURE);
    }
}

void clearStdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void registerUser() {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char hashed[MAX_PASSWORD_LEN];

    printf("Enter a username: ");
    secureInput(username, sizeof(username));
    printf("Enter a password: ");
    secureInput(password, sizeof(password));
    hashPassword(password, hashed);

    FILE *file = fopen(USER_FILE, "a");
    if (!file) {
        perror("[ERROR] Failed to open file");
        return;
    }
    fprintf(file, "%s:%s\n", username, hashed);
    fclose(file);
    printf("[INFO] User registered successfully.\n");
}

int loginUser() {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char hashed[MAX_PASSWORD_LEN];
    char line[MAX_LINE_LEN];

    printf("Enter username: ");
    secureInput(username, sizeof(username));
    printf("Enter password: ");
    secureInput(password, sizeof(password));
    hashPassword(password, hashed);

    FILE *file = fopen(USER_FILE, "r");
    if (!file) {
        printf("[INFO] No user data found. Please register first.\n");
        return 0;
    }

    while (fgets(line, sizeof(line), file)) {
        char storedUser[MAX_USERNAME_LEN], storedHash[MAX_PASSWORD_LEN];
        if (sscanf(line, "%31[^:]:%31s", storedUser, storedHash) == 2) {
            if (strcmp(username, storedUser) == 0 && strcmp(hashed, storedHash) == 0) {
                fclose(file);
                printf("[INFO] Login successful.\n");
                return 1;
            }
        }
    }

    fclose(file);
    printf("[INFO] User not found or password incorrect.\n");
    return 0;
}

void addEntry(struct Node **list, double *total, const char *type) {
    struct Node *newNode = malloc(sizeof(struct Node));
    if (!newNode) {
        fprintf(stderr, "[ERROR] Memory allocation failed.\n");
        return;
    }
    char date[MAX_DATE_LEN], category[MAX_CATEGORY_LEN];
    double amount;

    printf("Enter date (DD-MM-YYYY): ");
    secureInput(date, sizeof(date));
    printf("Enter amount: ");
    if (scanf("%lf", &amount) != 1 || amount < 0) {
        fprintf(stderr, "[ERROR] Invalid amount.\n");
        clearStdin();
        free(newNode);
        return;
    }
    clearStdin();
    printf("Enter category: ");
    secureInput(category, sizeof(category));

    strcpy(newNode->date, date);
    strcpy(newNode->category, category);
    newNode->amount = amount;
    newNode->next = NULL;

    if (!(*list)) {
        *list = newNode;
    } else {
        struct Node *ptr = *list;
        while (ptr->next)
            ptr = ptr->next;
        ptr->next = newNode;
    }

    *total += amount;
    printf("[INFO] %s entry added successfully.\n", type);
}

void addIncome() { addEntry(&incomeList, &currentIncome, "Income"); saveIncome(); }
void addExpense() { addEntry(&expenseList, &currentExpense, "Expense"); saveExpense(); }

void displayRecords(struct Node *list, const char *type) {
    if (!list) {
        printf("[INFO] No %s records to display.\n", type);
        return;
    }
    struct Node *ptr = list;
    printf("\n--- %s Records ---\n", type);
    while (ptr) {
        printf("Date: %s | Amount: %.2lf | Category: %s\n", ptr->date, ptr->amount, ptr->category);
        ptr = ptr->next;
    }
}

void saveToFile(struct Node *list, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) { perror("[ERROR] Cannot save to file"); return; }
    struct Node *ptr = list;
    while (ptr) {
        fwrite(ptr, sizeof(struct Node), 1, fp);
        ptr = ptr->next;
    }
    fclose(fp);
}

void loadFromFile(struct Node **list, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return;
    struct Node *head = NULL, *tail = NULL;
    struct Node temp;
    while (fread(&temp, sizeof(struct Node), 1, fp)) {
        struct Node *node = malloc(sizeof(struct Node));
        if (!node) break;
        *node = temp;
        node->next = NULL;
        if (!head) head = tail = node;
        else { tail->next = node; tail = node; }
    }
    fclose(fp);
    *list = head;
}

void saveIncome() { saveToFile(incomeList, INCOME_FILE); }
void saveExpense() { saveToFile(expenseList, EXPENSE_FILE); }
void loadIncome() { loadFromFile(&incomeList, INCOME_FILE); }
void loadExpense() { loadFromFile(&expenseList, EXPENSE_FILE); }

void saveRecord() {
    FILE *fp = fopen(RECORD_FILE, "wb");
    if (!fp) return;
    struct Record r = {currentIncome, currentExpense};
    fwrite(&r, sizeof(struct Record), 1, fp);
    fclose(fp);
}

void loadRecord() {
    FILE *fp = fopen(RECORD_FILE, "rb");
    if (!fp) return;
    struct Record r;
    fread(&r, sizeof(struct Record), 1, fp);
    currentIncome = r.income;
    currentExpense = r.expense;
    fclose(fp);
}

void freeList(struct Node *list) {
    struct Node *tmp;
    while (list) {
        tmp = list;
        list = list->next;
        free(tmp);
    }
}

void showMainMenu() {
    printf("\n==== Welcome to Finance Tracker ====");
    printf("\n1. Register\n2. Login\n3. Exit\nChoose an option: ");
}

void showFinanceMenu() {
    printf("\n==== Finance Dashboard ====");
    printf("\nTotal Income: %.2lf | Total Expense: %.2lf | Balance: %.2lf\n",
           currentIncome, currentExpense, currentIncome - currentExpense);
    printf("1. Add Income\n2. Add Expense\n3. View Income\n4. View Expense\n5. Logout\n");
    printf("Choose an option: ");
}

int main() {
    loadRecord();
    loadIncome();
    loadExpense();

    int choice, loggedIn = 0;

    while (!loggedIn) {
        showMainMenu();
        if (scanf("%d", &choice) != 1) {
            clearStdin();
            continue;
        }
        clearStdin();

        switch (choice) {
            case 1: registerUser(); break;
            case 2: loggedIn = loginUser(); break;
            case 3: exit(0);
            default: printf("[INFO] Invalid option.\n");
        }
    }

    do {
        showFinanceMenu();
        if (scanf("%d", &choice) != 1) {
            clearStdin();
            continue;
        }
        clearStdin();
        switch (choice) {
            case 1: addIncome(); break;
            case 2: addExpense(); break;
            case 3: displayRecords(incomeList, "Income"); break;
            case 4: displayRecords(expenseList, "Expense"); break;
            case 5: saveRecord(); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 5);

    freeList(incomeList);
    freeList(expenseList);
    printf("[INFO] Exiting Finance Tracker.\n");
    return 0;
}