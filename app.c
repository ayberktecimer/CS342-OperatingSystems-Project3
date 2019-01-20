#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int recursiveSum(int n) {
	if (n == 0){
		printf("Stack is maximum now. \n");
		printf("Check memory details. Press any key to complete recursion \n");
		getchar();
		
		return 0;
	}

	return n + recursiveSum(n - 1);
}
int main() {
	printf("PID: %d \n", getpid());

	printf("Check memory details. Press any key to allocate memory \n");
	getchar();
	
	char* array[50];
	for (int i = 0; i < 50; i++) {
		array[i] = malloc(100000);
	}

	printf("Allocation completed. \n");
	printf("Check memory details. Press any key to deallocate memory \n");
	getchar();

	for (int j = 0; j < 50; j++) {
		free(array[j]);
	}

	printf("Deallocation completed. \n");
	printf("Check memory details. Press any key to start recursion \n");
	getchar();

	recursiveSum(50000);

	printf("Recursive function completed. \n");
	printf("Check memory details. Press any key to exit \n");
	getchar();
	
	return 0;
}
