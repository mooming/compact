#include "SymbolDataTest.h"

#include "MeasureSec.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <random>

SymbolDataTest::SymbolDataTest()
   : passCount(0)
   , failCount(0)
{
}

bool SymbolDataTest::DoTest(int argc, const char* argv[])
{
   return TestWithSourceFile(argv[1], argv[2])
      && TestWithRandomData();
}

void SymbolDataTest::PrintSymbolData(const SymbolData& symData) const
{
   using namespace std;

   cout << endl;
   cout << "Data Length = " << symData.GetLength() << endl;
   cout << "Data Bytes = " << symData.GetDataBytes() << endl;

   {
      size_t length = symData.GetLength();
      for (size_t i = 0; i < length; ++i)
      {
         cout << static_cast<char>(symData[i]);
      }
      cout << endl;
   }

   cout << endl;
}

bool SymbolDataTest::CheckDataWithSourceFile(const char* srcPath, const SymbolData& symData) const
{
   using namespace std;

   ifstream ifs;
   ifs.open(srcPath, ios::binary);

   if (!ifs.is_open())
   {
      cerr << __FILE__ << ":" << __LINE__ << " [Warning] File not found. " << srcPath << endl;
      return false;
   }

   size_t index = 0;
   while (index < symData.GetLength())
   {
      auto ch = ifs.get();
      if (ch != symData[index++])
      {
         return false;
      }
   }

   return index == symData.GetLength();
}

bool SymbolDataTest::TestWithSourceFile(const char* loadPath, const char* savePath)
{
   using namespace std;

   cout << endl << "=== Load uncompressed data ===" << endl;
   auto symData = SymbolData::LoadUncompressed(loadPath);
   symData.Save(savePath);
   PrintSymbolData(symData);

   if (!CheckDataWithSourceFile(loadPath, symData))
   {
      cerr << "[Error] Data mismatched with the original source file." << endl;
      ++failCount;
   }
   else
   {
      ++passCount;
   }

   cout << endl << "=== Load compressed data ===" << endl;
   auto symData2 = SymbolData::LoadCompressed(savePath);

   if (symData != symData2)
   {
      cerr << "[Error] Data mismatched with the compressed file!" << endl;
      ++failCount;
   }
   else
   {
      ++passCount;
      float duration = 0;

      {
         MeasureSec measureSec(duration);

         size_t length = symData.GetLength();
         for (size_t i = 0; i < length; ++i)
         {
            if (symData[i] != symData2[i])
            {
               cerr << "[Error] data mismatched at " << i << endl;
               ++failCount;
            }
         }
      }
      cout << "Duration: " << duration << " sec" << endl;
   }

   PrintSymbolData(symData2);

   return failCount == 0;
}

bool SymbolDataTest::TestWithRandomData()
{
   using namespace std;

   cout << endl << "=== Load uncompressed data ===" << endl;

   size_t sizeInc = 1;
   vector<SymbolData::Symbol> randData;

   constexpr size_t testSize = 256 * 1024 * 1024;

   unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
   std::mt19937 randGen(seed);

   std::random_device device;
   std::mt19937 generator(device());
   std::uniform_int_distribution<int> setSizeDist(0, 51);

   for (size_t i = 0; i < testSize; i += sizeInc)
   {
      size_t indexOffset = 0;

      const int randStart = 'A';
      const int randEnd = 'A' + setSizeDist(generator);
      std::uniform_int_distribution<int> symbolDist(randStart, randEnd);

      while (randData.size() < i)
      {
         SymbolData::Symbol value = 0;
         if (i < 4096)
         {
            value = static_cast<SymbolData::Symbol>(symbolDist(generator));
         }
         else
         {
            value = randData[indexOffset++ % randData.size()];
         }

         randData.push_back(value);
      }

      SymbolData symData;

      float loadTime = 0;
      {
         MeasureSec measureSec(loadTime);
         symData = SymbolData::LoadUncompressed(randData.data(), randData.size());
      }
      cout << "Load & Compress Duration: " << loadTime << " sec, size = " << randData.size() << endl;
      if (loadTime > 5.0f)
         break;

      // Check with the source data(randData)
      // Measure iteration time
      ++passCount;

      size_t length = symData.GetLength();
      float duration = 0;

      if (length < 1024)
      {
         PrintSymbolData(symData);
      }

      {
         MeasureSec measureSec(duration);
         for (size_t i = 0; i < length; ++i)
         {
            if (symData[i] != randData[i])
            {
               cerr << "[Error] data mismatched at " << i << endl;
               ++failCount;
            }
         }
      }
      cout << "Duration: " << duration << " sec, size = " << length << endl;
      ++passCount;

      if (i > 16)
         sizeInc *= 2;
   }

   return failCount == 0;
}
