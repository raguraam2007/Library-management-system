#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <cstdio>
using namespace std;

struct Book {
    int id;
    string title;
    string author;
    int total;
    int available;
};

const string BOOKS_FILE = "books.txt";
const string ISSUES_FILE = "issued.txt";

string lowerText(string text) {
    transform(text.begin(), text.end(), text.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return text;
}

bool readBook(ifstream& file, Book& book) {
    return static_cast<bool>(file >> book.id >> quoted(book.title)
                             >> quoted(book.author) >> book.total
                             >> book.available);
}

bool bookExists(int id) {
    ifstream file(BOOKS_FILE);
    Book book;
    while (readBook(file, book)) {
        if (book.id == id) return true;
    }
    return false;
}

void addBook() {
    Book book;
    cout << "\nEnter Book ID: ";
    if (!(cin >> book.id) || book.id <= 0) {
        cout << "Please enter a valid positive ID.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    if (bookExists(book.id)) {
        cout << "A book with that ID already exists.\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter Book Title: ";
    getline(cin, book.title);
    cout << "Enter Author Name: ";
    getline(cin, book.author);

    if (book.title.empty() || book.author.empty()) {
        cout << "Title and author cannot be empty.\n";
        return;
    }

    cout << "Enter Number of Copies: ";
    if (!(cin >> book.total) || book.total <= 0) {
        cout << "Number of copies must be greater than zero.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    book.available = book.total;
    ofstream file(BOOKS_FILE, ios::app);
    if (!file) {
        cout << "Could not open the books file.\n";
        return;
    }

    file << book.id << ' ' << quoted(book.title) << ' '
         << quoted(book.author) << ' ' << book.total << ' '
         << book.available << '\n';

    cout << "Book added successfully!\n";
}

void displayBooks() {
    ifstream file(BOOKS_FILE);
    if (!file) {
        cout << "\nNo books found. Add a book first.\n";
        return;
    }

    Book book;
    bool found = false;
    cout << "\n================ BOOK LIST ================\n";
    cout << left << setw(8) << "ID" << setw(28) << "TITLE"
         << setw(24) << "AUTHOR" << setw(10) << "TOTAL"
         << "AVAILABLE\n";
    cout << string(82, '-') << '\n';

    while (readBook(file, book)) {
        cout << left << setw(8) << book.id
             << setw(28) << book.title.substr(0, 26)
             << setw(24) << book.author.substr(0, 22)
             << setw(10) << book.total
             << book.available << '\n';
        found = true;
    }

    if (!found) cout << "No books found.\n";
}

void searchBooks() {
    int option;
    string term;
    cout << "\nSearch by:\n1. Title\n2. Author\nEnter choice: ";
    if (!(cin >> option) || (option != 1 && option != 2)) {
        cout << "Invalid choice.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter search text: ";
    getline(cin, term);
    term = lowerText(term);

    ifstream file(BOOKS_FILE);
    Book book;
    bool found = false;

    while (readBook(file, book)) {
        const string field = lowerText(option == 1 ? book.title : book.author);
        if (field.find(term) != string::npos) {
            cout << "\nBook ID: " << book.id
                 << "\nTitle: " << book.title
                 << "\nAuthor: " << book.author
                 << "\nTotal Copies: " << book.total
                 << "\nAvailable Copies: " << book.available << "\n";
            found = true;
        }
    }

    if (!found) cout << "No matching books found.\n";
}

bool updateBookAvailability(int id, int change) {
    ifstream input(BOOKS_FILE);
    if (!input) {
        cout << "No book records found.\n";
        return false;
    }

    const string tempName = "books_temp.txt";
    ofstream output(tempName);
    if (!output) {
        cout << "Could not create temporary file.\n";
        return false;
    }

    Book book;
    bool found = false;
    bool valid = true;

    while (readBook(input, book)) {
        if (book.id == id) {
            found = true;
            if (change < 0 && book.available == 0) {
                cout << "No copies of this book are currently available.\n";
                valid = false;
                break;
            }
            if (change > 0 && book.available >= book.total) {
                cout << "All copies are already marked as available.\n";
                valid = false;
                break;
            }
            book.available += change;
        }

        output << book.id << ' ' << quoted(book.title) << ' '
               << quoted(book.author) << ' ' << book.total << ' '
               << book.available << '\n';
    }

    input.close();
    output.close();

    if (!valid) {
        remove(tempName.c_str());
        return false;
    }

    if (!found) {
        remove(tempName.c_str());
        cout << "Book ID not found.\n";
        return false;
    }

    if (remove(BOOKS_FILE.c_str()) != 0) {
        remove(tempName.c_str());
        cout << "Could not replace the books file.\n";
        return false;
    }

    if (rename(tempName.c_str(), BOOKS_FILE.c_str()) != 0) {
        cout << "Could not save the updated book records.\n";
        return false;
    }

    return true;
}

void issueBook() {
    int id;
    string memberName;

    cout << "\nEnter Book ID to issue: ";
    if (!(cin >> id) || id <= 0) {
        cout << "Invalid book ID.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter Member Name: ";
    getline(cin, memberName);
    if (memberName.empty()) {
        cout << "Member name cannot be empty.\n";
        return;
    }

    if (!updateBookAvailability(id, -1)) return;

    ofstream file(ISSUES_FILE, ios::app);
    if (!file) {
        // Attempt to restore availability if the issue record cannot be saved.
        updateBookAvailability(id, 1);
        cout << "Could not save the issue record.\n";
        return;
    }

    file << id << ' ' << quoted(memberName) << '\n';
    cout << "Book issued successfully to " << memberName << ".\n";
}

void returnBook() {
    int id;
    string memberName;

    cout << "\nEnter Book ID to return: ";
    if (!(cin >> id) || id <= 0) {
        cout << "Invalid book ID.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter Member Name on the issue record: ";
    getline(cin, memberName);

    ifstream input(ISSUES_FILE);
    if (!input) {
        cout << "No issued-book records found.\n";
        return;
    }

    const string tempName = "issued_temp.txt";
    ofstream output(tempName);
    if (!output) {
        cout << "Could not create temporary issue file.\n";
        return;
    }

    int issuedId;
    string issuedMember;
    bool found = false;

    while (input >> issuedId >> quoted(issuedMember)) {
        if (!found && issuedId == id && issuedMember == memberName) {
            found = true;
            continue;
        }
        output << issuedId << ' ' << quoted(issuedMember) << '\n';
    }

    input.close();
    output.close();

    if (!found) {
        remove(tempName.c_str());
        cout << "No matching issue record was found.\n";
        return;
    }

    if (!updateBookAvailability(id, 1)) {
        remove(tempName.c_str());
        cout << "Could not update book availability; return was not completed.\n";
        return;
    }

    if (remove(ISSUES_FILE.c_str()) != 0 ||
        rename(tempName.c_str(), ISSUES_FILE.c_str()) != 0) {
        cout << "Warning: could not finalize the issue record file. Please check records.\n";
        return;
    }

    cout << "Book returned successfully!\n";
}

int main() {
    int choice;

    do {
        cout << "\n========================================\n"
             << "       LIBRARY MANAGEMENT SYSTEM\n"
             << "========================================\n"
             << "1. Add Book\n"
             << "2. Display All Books\n"
             << "3. Search by Title or Author\n"
             << "4. Issue Book\n"
             << "5. Return Book\n"
             << "6. Exit\n"
             << "========================================\n"
             << "Enter your choice: ";

        if (!(cin >> choice)) {
            cout << "Please enter a valid number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1: addBook(); break;
            case 2: displayBooks(); break;
            case 3: searchBooks(); break;
            case 4: issueBook(); break;
            case 5: returnBook(); break;
            case 6: cout << "Thank you for using the Library Management System!\n"; break;
            default: cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 6);

    return 0;
}
