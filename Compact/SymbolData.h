#pragma once

#include <cstdint>
#include <string>
#include <vector>

class SymbolData final
{
public:
   using PathString = std::string;
   using Symbol = uint8_t;
   using Index = uint8_t;
   using DataUnit = size_t;

public:
   static SymbolData LoadUncompressed(Symbol* data, size_t length);
   static SymbolData LoadUncompressed(const PathString& path);
   static SymbolData LoadCompressed(const PathString& path);

   SymbolData();
   Symbol operator[] (size_t index) const { return Get(index); }
   bool operator== (const SymbolData& rhs) const;
   bool operator!= (const SymbolData& rhs) const { return !(*this == rhs); }

   Symbol Get(size_t index) const;
   size_t GetLength() const { return length; }
   size_t GetDataBytes() const { return binData.size() * sizeof(DataUnit); }

   std::vector<Symbol> GetDecompressed() const;
   bool Save(const PathString& path) const;

private:
   void Compress(const std::vector<Symbol>& symTable, Symbol* symData, size_t length);

private:
   size_t length;
   Symbol mask;
   uint8_t numSymBits;

   std::vector<Symbol> symTable;
   std::vector<DataUnit> binData;
};
