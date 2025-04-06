#include <iostream>
#include <vector>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    int sum = 0;

    for (int number : numbers) {
        sum += number;
    }

    std::cout << "The sum is: " << sum << std::endl;
    return 0;
}