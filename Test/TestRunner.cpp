// Unity build test runner - Handmade Hero style
// All tests compiled in a single translation unit

#include "test.h"
#include "Memory.h"

// Global arena for tests that need it
MemoryArena* arena;

// Include all test files
#include "Src/TestArray.cpp"
#include "Src/TestLinAlg.cpp"
#include "Src/TestString.cpp"
#include "Src/TestThreadPool.cpp"

int main(int argc, char** argv)
{
    // Initialize global test resources
    arena = CreateMemoryArena(MegaBytes(16));

    printf("Running AnymaEngine Tests...\n");

    // Tests run automatically via static constructors
    // Return test results
    int result = RunTests();

    return result;
}
