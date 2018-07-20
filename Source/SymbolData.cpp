#include "SymbolData.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <set>

namespace
{
   using DataUnit = SymbolData::DataUnit;

   DataUnit ReadBitsMasked(DataUnit value, int offset, DataUnit mask, int length)
   {
      constexpr int numBits = sizeof(SymbolData::DataUnit) * 8;

      int shift = numBits - offset - length;
      if (shift >= 0)
      {
         mask = mask << shift;
         value &= mask;
         value = value >> shift;
      }
      else
      {
         shift = -shift;
         mask = mask >> shift;
         value &= mask;
         value = value << shift;
      }

      return value;
   }

   DataUnit ReadBits(DataUnit value, int offset, int length)
   {
      constexpr int numBits = sizeof(SymbolData::DataUnit) * 8;
      DataUnit mask = std::numeric_limits<DataUnit>::max() >> (numBits - length);

      int shift = numBits - offset - length;
      if (shift >= 0)
      {
         mask = mask << shift;
         value &= mask;
         value = value >> shift;
      }
      else
      {
         shift = -shift;
         mask = mask >> shift;
         value &= mask;
         value = value << shift;
      }

      return value;
   }
}

SymbolData SymbolData::LoadUncompressed(Symbol* symData, size_t length)
{
   using namespace std;

   SymbolData compact;
   set<Symbol> symSet;

   for (size_t i = 0; i < length; ++i)
   {
      symSet.insert(symData[i]);
   }

   vector<Symbol> symTable;
   symTable.reserve(symSet.size());

   for (const auto symbol : symSet)
   {
      symTable.push_back(symbol);
   }

   compact.Compress(symTable, symData, length);

   return compact;
}

SymbolData SymbolData::LoadUncompressed(const PathString& path)
{
   using namespace std;

   SymbolData compact;

   ifstream ifs;
   ifs.open(path.c_str(), ios::binary);

   if (!ifs.is_open())
   {
      cerr << __FILE__ << ":" << __LINE__ << " [Warning] File not found. " << path << endl;
      return compact;
   }

   ifs.seekg(0, ifs.end);
   streamsize length = ifs.tellg();
   ifs.seekg(0, ifs.beg);

   if (length == 0)
   {
      cerr << __FILE__ << ":" << __LINE__ << " [Warning] empty data " << path << endl;
      return compact;
   }

   Symbol* symData = new Symbol[length];
   ifs.read(reinterpret_cast<char*>(&symData[0]), length);
   ifs.close();

   compact = LoadUncompressed(symData, length);
   cout << "Load: " << path << " [raw][done] " << length << " bytes" << endl;

   return compact;
}

SymbolData SymbolData::LoadCompressed(const PathString & path)
{
   using namespace std;

   SymbolData compact;

   ifstream ifs;
   ifs.open(path.c_str(), ios::binary);

   if (!ifs.is_open())
   {
      cerr << __FILE__ << ":" << __LINE__ << " [Warning] Failed to open. " << path << endl;
      return compact;
   }

   ifs.read(reinterpret_cast<char*>(&compact.length), sizeof(decltype(compact.length)));
   if (compact.length == 0)
   {
      cerr << __FILE__ << ":" << __LINE__ << " [Warning] empty symbol data." << endl;
      return compact;
   }

   ifs.read(reinterpret_cast<char*>(&compact.mask), sizeof(decltype(compact.mask)));
   if (compact.mask == 0)
   {
      cerr << __FILE__ << ":" << __LINE__ << " [Warning] null mask." << endl;
      return compact;
   }

   ifs.read(reinterpret_cast<char*>(&compact.numSymBits), sizeof(decltype(compact.numSymBits)));
   if (compact.numSymBits == 0)
   {
      cerr << __FILE__ << ":" << __LINE__ << " [Warning] number of symbol bits is zero." << endl;
      return compact;
   }

   size_t tableLength = 0;
   ifs.read(reinterpret_cast<char*>(&tableLength), sizeof(decltype(tableLength)));
   if (tableLength == 0)
   {
      cerr << __FILE__ << ":" << __LINE__ << " [Warning] empty symbol table." << endl;
      return compact;
   }

   compact.symTable.reserve(tableLength);
   for (size_t i = 0; i < tableLength; ++i)
   {
      Symbol symbol = 0;
      ifs.read(reinterpret_cast<char*>(&symbol), sizeof(Symbol));
      compact.symTable.push_back(symbol);
   }


   size_t dataLength = 0;
   ifs.read(reinterpret_cast<char*>(&dataLength), sizeof(size_t));

   if (dataLength == 0)
   {
      compact.symTable.clear();

      cerr << __FILE__ << ":" << __LINE__ << " [Warning] empty symbol data." << endl;
      return compact;
   }

   auto& data = compact.binData;
   data.reserve(dataLength);

   for (size_t i = 0; i < dataLength; ++i)
   {
      DataUnit value = 0;
      ifs.read(reinterpret_cast<char*>(&value), sizeof(DataUnit));
      data.push_back(value);
   }

   ifs.close();

   cout << "Loaded: " << path << " [compressed][done]" << endl;
   return compact;
}

SymbolData::SymbolData()
   : symTable()
   , binData()
{
}

bool SymbolData::operator==(const SymbolData& rhs) const
{
   if (length != rhs.length)
      return false;

   if (mask != rhs.mask)
      return false;

   if (numSymBits != rhs.numSymBits)
      return false;

   if (symTable != rhs.symTable)
      return false;

   if (binData != rhs.binData)
      return false;

   return true;
}

SymbolData::Symbol SymbolData::Get(size_t index) const
{
   Symbol symbol = 0;

   constexpr size_t numUnitBits = sizeof(DataUnit) * 8;

   const size_t bitIndex = index * numSymBits;
   const size_t unitIndex = bitIndex / numUnitBits;
   const size_t offset = bitIndex - (unitIndex * numUnitBits);

   DataUnit unitValue = binData[unitIndex];

   if ((offset + numSymBits) <= numUnitBits)
   {
      symbol = static_cast<Symbol>(ReadBitsMasked(unitValue, static_cast<int>(offset), mask, numSymBits));
   }
   else
   {
      auto nextUnit = binData[unitIndex + 1];
      symbol = static_cast<Symbol>(ReadBits(unitValue, static_cast<int>(offset), numSymBits));

      const size_t truncatedBit = offset + numSymBits - numUnitBits;
      symbol |= static_cast<Symbol>(ReadBits(nextUnit, 0, static_cast<int>(truncatedBit)));
   }

   assert(static_cast<size_t>(symbol) < symTable.size());

   return symTable[symbol];
}

void SymbolData::Compress(const std::vector<Symbol>& symTable, Symbol * symData, size_t dataLength)
{
   using namespace std;
   SymbolData::symTable = symTable;
   length = dataLength;

   auto NextPowerOf2 = [](size_t value) -> size_t
   {
      size_t power = 2;
      while (power < value)
         power *= 2;

      return power;
   };

   const auto numSyms = symTable.size();
   const auto p2 = NextPowerOf2(symTable.size());
   cout << "# Next power of 2 = " << p2 << ", where number of unique symbols = " << numSyms << endl;

   mask = static_cast<Symbol>(p2 - 1);
   cout << "# Symbol Mask = " << std::hex << (int)mask << std::dec << endl;

   numSymBits = 0;

   {
      Symbol tmpMask = mask;
      while (tmpMask != 0)
      {
         if (tmpMask & 0x01)
            ++numSymBits;

         tmpMask = tmpMask >> 1;
      }
   }

   cout << "# Number of symbol bits " << (int)numSymBits << endl;

   auto GetIndex = [&symTable](Symbol symbol) -> size_t
   {
      const auto length = symTable.size();
      for (size_t i = 0; i < length; ++i)
      {
         if (symbol == symTable[i])
            return i;
      }

      return -1;
   };

   constexpr auto numUnitBits = sizeof(DataUnit) * 8;
   const size_t compactBitsSize = dataLength * numSymBits;

   size_t unitDataLength = compactBitsSize / numUnitBits;
   if ((compactBitsSize % numUnitBits) > 0)
      ++unitDataLength;

   binData.reserve(unitDataLength);

   {
      DataUnit bitsToRead = numUnitBits;
      DataUnit unit = 0;

      for (size_t i = 0; i < dataLength; ++i)
      {
         auto symbol = symData[i];
         const DataUnit symInd = GetIndex(symbol);

         if (bitsToRead < numSymBits)
         {
            if (bitsToRead == 0)
            {
               binData.push_back(unit);
               unit = symInd;
               bitsToRead = numUnitBits - numSymBits;
            }
            else
            {
               const uint8_t truncatedBits = numSymBits - static_cast<uint8_t>(bitsToRead);
               unit = unit << bitsToRead;

               const DataUnit truncated = symInd >> truncatedBits;
               unit |= truncated;

               binData.push_back(unit);

               unit = symInd & (mask >> (numSymBits - truncatedBits));
               bitsToRead = numUnitBits - truncatedBits;

            }
         }
         else
         {
            unit = unit << numSymBits;
            unit |= (mask & symInd);

            bitsToRead -= numSymBits;
         }
      }

      // Store remained data
      if (bitsToRead < numUnitBits)
      {
         unit = unit << bitsToRead;
         binData.push_back(unit);

         unit = 0;
         bitsToRead = numUnitBits;
      }
   }

   float compRate = (1.0f - (float)GetDataBytes() / GetLength()) * 100.0f;
   cout << "# Compress Rate = " << compRate << "%, " << GetLength() << " => " << GetDataBytes()  << endl;
}

bool SymbolData::Save(const PathString& path) const
{
   using namespace std;

   ofstream ofs;
   ofs.open(path.c_str(), ios::binary);

   if (!ofs.is_open())
   {
      cerr << "[Error] " << __FILE__ << ":" << __LINE__ << "Failed to open a file. " << path << endl;
      return false;
   }

   ofs.write(reinterpret_cast<const char*>(&length), sizeof(decltype(length)));
   ofs.write(reinterpret_cast<const char*>(&mask), sizeof(decltype(mask)));
   ofs.write(reinterpret_cast<const char*>(&numSymBits), sizeof(decltype(numSymBits)));

   auto tableLength = symTable.size();
   ofs.write(reinterpret_cast<const char*>(&tableLength), sizeof(decltype(tableLength)));

   for (auto symbol : symTable)
   {
      ofs.write(reinterpret_cast<const char*>(&symbol), sizeof(decltype(symbol)));
   }

   auto dataLength = binData.size();
   ofs.write(reinterpret_cast<const char*>(&dataLength), sizeof(decltype(dataLength)));

   for (auto value : binData)
   {
      ofs.write(reinterpret_cast<const char*>(&value), sizeof(decltype(value)));
   }

   ofs.close();

   return true;
}
