#ifndef HELPER_H
#define HELPER_H

#ifdef NDEBUG
#define PRINT(a, b)
#define ERROR(a)
#else
#define PRINT(a, b) std::cout << "[DEBUG]: " << (a) << (b) << std::endl;
#define ERROR(a) std::cerr << "[ERROR]: " << (a) << std::endl

#endif

#endif	// HELPER_H