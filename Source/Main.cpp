#include "SymbolDataTest.h"

#include <iostream>

void PrintArgs(int argc, const char* argv[])
{
   using namespace std;

   int argIndex = 0;
   cout << argv[argIndex++];

   while (argc > argIndex)
   {
      cout << " " << argv[argIndex++];
   }

   cout << endl;
}


int main(int argc, const char* argv[])
{
   using namespace std;

   PrintArgs(argc, argv);

   if (argc != 3)
   {
      cout << "Usage: " << argv[0] << " <path to read the uncompressed data>"
         << " <path to save the compressed data>" << endl;
      return 0;
   }

   SymbolDataTest sdTest;
   if (!sdTest.DoTest(argc, argv))
   {
      cerr << endl << "[Result] Test failed." << endl;
      cerr << "Pass : " << sdTest.GetPassCount() << endl;
      cerr << "Fail : " << sdTest.GetFailCount() << endl;

      return -1;
   }

   cerr << endl << "[Result] Test has succeeded." << endl;
   cerr << "Pass : " << sdTest.GetPassCount() << endl;
   cerr << "Fail : " << sdTest.GetFailCount() << endl;

   return 0;
}