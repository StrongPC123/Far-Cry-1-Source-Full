#include <string>

      int main()
      {
      std::string str1 = "string";
      std::string str2;
      str2.assign(str1.begin(), str1.begin() + 3);
      return 0;
      }
