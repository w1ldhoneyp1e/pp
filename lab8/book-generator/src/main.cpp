#include "BookGenerator.h"

#include <iostream>
#include <vector>

int main()
{
    std::vector<Book> books = {
        { "The Great Gatsby", "F. Scott Fitzgerald", { "Chapter 1", "Chapter 2" } },
        { "1984", "George Orwell", { "Chapter 1", "Chapter 2", "Chapter 3" } },
        { "To Kill a Mockingbird", "Harper Lee", { "Chapter 1" } },
    };

    for (const BookChapter& chapter : ListBookChapters(books)) {
        std::cout << chapter << std::endl;
    }

    return 0;
}
