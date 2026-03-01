#include "executor.h"

void execute(Node *node)
{
#if _WIN32
    execute_win(node);
#else

#endif
}