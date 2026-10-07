#include <stdio.h>
#include <assert.h>

#include "bookmark.h"

int main(void)
{
    bookmarkInit();
    assert(bookmarkCount() == 0);
    assert(bookmarkIsBookmarked(5) == 0);

    /* Toggle bookmark at line 5 */
    int res = bookmarkToggle(5);
    assert(res == 1); /* Added */
    assert(bookmarkCount() == 1);
    assert(bookmarkIsBookmarked(5) == 1);

    /* Toggle again removes it */
    res = bookmarkToggle(5);
    assert(res == 0); /* Removed */
    assert(bookmarkCount() == 0);
    assert(bookmarkIsBookmarked(5) == 0);

    /* Add multiple in random order: 10, 2, 20 */
    bookmarkToggle(10);
    bookmarkToggle(2);
    bookmarkToggle(20);
    assert(bookmarkCount() == 3);

    /* Check next navigation */
    assert(bookmarkNext(0) == 2);
    assert(bookmarkNext(2) == 10);
    assert(bookmarkNext(10) == 20);
    assert(bookmarkNext(25) == 2); /* Wrap around */

    /* Check prev navigation */
    assert(bookmarkPrev(25) == 20);
    assert(bookmarkPrev(20) == 10);
    assert(bookmarkPrev(10) == 2);
    assert(bookmarkPrev(1) == 20); /* Wrap around */

    /* Clear all */
    bookmarkClearAll();
    assert(bookmarkCount() == 0);
    assert(bookmarkNext(0) == -1);

    printf("bookmark_test passed!\n");
    return 0;
}
