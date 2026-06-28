#include <memory>
#include <random>
#include <vector>
#include <map>
#include <cassert>
#include <sstream>
#include <iostream>

#include "CommonTestFunctionality.h"
#include "Huffman.h"

class HuffmanTest : public Test
{
protected:

	virtual void SetUp()
	{
	}

	virtual void TearDown()
	{
	}
};

TEST_F(HuffmanTest, FirstTest)
{
	char* data_in = "aaabbcab";
	char data_out[32];
	Huffman::Compressor c;
	c.Work(data_in, strlen(data_in));
	size_t s = c.Flush(data_out,32);
	Huffman::Decompressor d;
	d.Work(data_out, s);

}
