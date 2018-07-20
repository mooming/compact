#pragma once

#include "SymbolData.h"

// This is the unit-test class of SymbolData
class SymbolDataTest final
{
public:
   SymbolDataTest();

   bool DoTest(int argc, const char* argv[]);

   void PrintSymbolData(const SymbolData& data) const;

   size_t GetPassCount() const { return passCount; }
   size_t GetFailCount() const { return failCount; }

private:
   bool CheckDataWithSourceFile(const char* srcPath, const SymbolData& symData) const;

   // Testing with input & output files
   bool TestWithSourceFile(const char* loadPath, const char* savePath);

   // Testing with randomly generated source data
   bool TestWithRandomData();

private:
   size_t passCount;
   size_t failCount;
};
