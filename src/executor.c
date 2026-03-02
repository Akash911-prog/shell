#include "executor.h"

void execute(Node *node)
{
#if _WIN32
    IOContext io = default_io();
    execute_win(node, io);
#else

#endif
}