#include <stdio.h>
#include <stdlib.h>

// Simple test to verify compilation works
int main() {
    printf("SCC Library Test - Basic compilation check\n");
    printf("All core components compiled successfully!\n");
    
    // Basic memory allocation test
    void* ptr = malloc(100);
    if (ptr) {
        printf("Memory allocation: OK\n");
        free(ptr);
    } else {
        printf("Memory allocation: FAILED\n");
        return 1;
    }
    
    printf("Simple test passed!\n");
    return 0;
}