#pragma once

#include "Generator.h"

#include <iostream>
#include <string>
#include <vector>

struct Book {
    std::string title;
    std::string author;
    std::vector<std::string> chapters;
};

struct BookChapter {
    std::string bookTitle;
    std::string bookAuthor;
    std::string chapterTitle;
};

std::ostream& operator<<(std::ostream& os, const BookChapter& chapter)
{
    return os << chapter.bookTitle << " by " << chapter.bookAuthor << ": "
              << chapter.chapterTitle;
}

Generator<BookChapter> ListChapters(const Book& book)
{
    for (const std::string& chapterTitle : book.chapters) {
        co_yield BookChapter {
            book.title,
            book.author,
            chapterTitle,
        };
    }
}

Generator<BookChapter> ListBookChapters(const std::vector<Book>& books)
{
    for (const Book& book : books) {
        for (const BookChapter& chapter : ListChapters(book)) {
            co_yield chapter;
        }
    }
}
