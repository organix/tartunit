# tartunit

Unit test framework for Tiny Actor Run-Time.

## Tests

    ~$ cd src
    ~/src$ make test

## Examples

```c
#define  TARTUNIT /* turn tests on */
#include "actor.h"
#include "tartunit.h"

int
main()
{
    Actor a_my_ignore = actor_new(beh_ignore);
    TEST(
        EXPECT_EVENT(a_my_ignore);
        TEST_SEND(a_my_ignore, (Any)0);
    );
}
```