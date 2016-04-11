#include <random>
#include <iostream>

int main()
{
    std::mt19937 rng;
    rng.seed(std::random_device()());
    // distribution in range [1, 6]
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,6);

    std::cout << dist6(rng) << std::endl;
}

