Programming Task;

## Conditions

1. Probability is equally distributed.
	=> every character has the same count of its appearance.
	=> Worst case in Huffman algorithm.
	
2. Number of unique characters are defined, for example n = 16.
	=> Set of unique symbols could have a various size.

3. Symbols occur in non-repeating order.
	=> Run-length algorithm is bad to do this.
	
4. Constant look-up time => O(1)
	=> Almost compression algorithm fails due to this condition.
	=> Every element should be located at a predictable position.


## Design

Because of the condition 4,
I didn't apply any compress algorithm to store it with a compact size.

Each element occupies minimum bits which span the unique set of symbols instead of occupying whole character.
Since std::vector<bool> is a possibly space-efficient specialization,
it is good candidate to do that.
But it depends on its implementation and has more burden to save and load data because it works bitwise manner only.

This project has its own implementation of collection of symbol to reduce unused bits.
Compression rate linearly depends on the size of unique symbol set.
Hence the worst case is # of unique symbols = 255.

This project contains 2 unit-tests.
1. Testing with a source file and output file.
2. Testing with a randomly generated data.

Outcome file doesn't have ID, version, checksum.
Thus it could cause an expected behavior if its data varied by accident.


## How to use

Usage:
Usage: Compact <path to read the uncompressed data> <path to save the compressed data>

Output Example:
[Result] Test has succeeded.
Pass : 74
Fail : 0

Tested Edge cases:
	Input data size = 0
	Input data size = 1
	Number of unique symbols = 0
	Number of unique symbols = 1
	Number of unique symbols = 256
	

## How to build
Use Cmake to make a proper project on current platform.
Then just build it.
