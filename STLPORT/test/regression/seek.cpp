

// Regression test for STL seekg

#include <cstdio>       // I really like formatted output.
#include <cstdarg>
#include <fstream>
#include <vector>

#ifndef cstd
#ifdef _MSC_VER
#define cstd
#else
#define cstd std
#endif
#endif

#ifndef USING_STLPORT
#define vsnprintf _vsnprintf
#endif

class Error : public std::exception
{
 public:
    Error(const char *pszfmt, ...);
    const char *what() const;
 private:
    char mMessage[256];
};

#define countof(x) (sizeof(x)/sizeof((x)[0]))

Error::Error(const char *pszfmt, ...)
{
    cstd::va_list ap;
    va_start(ap, pszfmt);
    cstd::vsnprintf(mMessage, countof(mMessage), pszfmt, ap);
    va_end(ap);
}

const char *Error::what() const
{
    return mMessage;
}

// usage: testseek filename
int main(int argc, char **argv)
{
    try
    {
        if (argc != 2) {
            throw Error("Wrong number of arguments (Usage: testseek filename)");
        }
        std::ifstream ifs(argv[1]);
        if (!ifs) {
            throw Error("Failed to open file \"%s\"", argv[1]);
        }

        ifs.exceptions(std::ios::badbit);
        // First pass: read contents of file into character vector
        std::vector<char> charvec;
        cstd::printf("First pass:\n");
        for(;;)
        {
            char ch;
            if (!ifs.get(ch)) {
                break;
            }
            cstd::printf("%c", ch);
            charvec.push_back(ch);
        }
        printf("First pass done\n");

        // Second pass: get positions into pos vector, checking chars
        std::vector<std::ios::pos_type> posvec;
        ifs.clear();    // necessary?
        ifs.seekg(0);
        std::vector<char>::const_iterator char_it;
        printf("Second pass:\n");
        for(char_it = charvec.begin(); char_it != charvec.end(); ++char_it)
        {
            std::ios::pos_type pos = ifs.tellg();
            posvec.push_back(pos);
            char ch;
            if (!ifs.get(ch)) {
                break;
            }
            cstd::printf("%c", ch);
            if (ch != *char_it) {
                throw Error("Character mismatch: got '%c', expected '%c'",
                            ch, *char_it);
            }
        }
        if (char_it != charvec.end()) {
            throw Error("Unexpected end of file");
        }

        printf("Second pass done\n");

        // Third pass: seek to each saved position and read next char
        ifs.clear();    // necessary, but I don't like it
        std::vector<std::ios::pos_type>::const_iterator pos_it;

        for(pos_it = posvec.begin();
            pos_it != posvec.end(); ++pos_it)
        {
            printf ("%d ", (int)*pos_it);
        }
        printf ("\n");

        printf("Third pass:\n");
        for(char_it = charvec.begin(), pos_it = posvec.begin();
            char_it != charvec.end(); ++char_it, ++pos_it)
        {
            ifs.seekg(*pos_it);
            char ch;
            if (!ifs.get(ch)) {
                throw Error("End of file when expecting '%c'", *char_it);
            }
            cstd::printf("%c", ch);
            if (ch != *char_it) {
                throw Error("Character mismatch: got '%c', expected '%c'",
                            ch, *char_it);
            }
        }
        printf("Third pass done\n");
        printf("Test passed\n");
        
            
    }
    catch (std::exception &e)
    {
        cstd::printf("\nCaught exception: %s\n", e.what());
        cstd::printf("Test failed!\n");
        return 1;
    }
    return 0;
}
