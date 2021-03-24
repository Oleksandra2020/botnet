#include <iostream>
#include <ostream>
#include <string>


int main(int argc, char* argv[]) {
        if (argc < 2) {
                std::cerr << "FEWER ARGUMENTS THAN EXPECTED" << std::endl;
                return -1;
        }

        std::string test_argument = argv[1];
        std::cout << test_argument << std::endl;

	return 0;
}
