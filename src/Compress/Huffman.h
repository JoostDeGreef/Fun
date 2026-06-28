#pragma once

#include <algorithm>
#include <vector>
#include <cassert>

using namespace std;

#include "BitFiFo.h"

class Huffman
{
private:
	typedef int index_type;
	typedef int count_type;
	typedef int value_type;
	typedef int length_type;
	typedef uint64_t bits_type;

	class Bits
	{
	public:
		Bits()
		{}
		Bits(length_type length, bits_type bits)
			: m_length(length)
			, m_bits(bits)
		{}
		void Invalidate()
		{
			m_length = -1;
			m_bits = 0;
		}
		length_type GetLength() const
		{
			return m_length;
		}
		bits_type GetBits() const
		{
			return m_bits;
		}
		void AddBit(bool bit)
		{
			m_length++;
			m_bits <<= 1;
			if (bit)
			{
				m_bits |= 1;
			}
		}
	private:
		length_type m_length;
		bits_type m_bits;
	};

	class Node
	{
	public:
		Node()
			: Node(-1, -1, -1, -1)
		{}
		Node(count_type count, index_type parent, index_type left, index_type right)
			: Node(count, -1, parent, left, right)
		{}
		Node(count_type count, value_type value, index_type parent, index_type left, index_type right)
			: Node(-1, 0, count, value, parent, left, right)
		{}
		Node(length_type length, bits_type bits, count_type count, value_type value, index_type parent, index_type left, index_type right)
			: Node(Bits(length, bits), count, value, parent, left, right)
		{}
		Node(Bits bits, count_type count, value_type value, index_type parent, index_type left, index_type right)
			: m_bits(bits)
			, m_count(count)
			, m_value(value)
			, m_parent(parent)
			, m_left(left)
			, m_right(right)
		{}
		const Bits& GetBits() const { assert(m_bits.GetLength() >= 0); return m_bits; }
		Bits& GetBits() { return m_bits; }
		count_type GetCount() const { return m_count; }
		value_type GetValue() const { return m_value; }
		index_type GetParent() const { return m_parent; }
		index_type GetLeft() const { return m_left; }
		index_type GetRight() const { return m_right; }
		void InvalidateBits() { SetBits(-1, 0); }
		void SetBits(const length_type length, const bits_type bits) { SetBits(Bits(length, bits)); }
		void SetBits(const Bits& bits) { m_bits = bits; }
		void SetCount(const count_type count) { m_count = count; }
		void SetValue(const value_type value) { m_value = value; }
		void SetParent(const index_type parent) { m_parent = parent; }
		void SetLeft(const index_type left) { m_left = left; }
		void SetRight(const index_type right) { m_right = right; }

	private:
		Bits m_bits;
		count_type m_count;
		value_type m_value;
		index_type m_parent;
		index_type m_left;
		index_type m_right;
	};

	class Table
	{
	public:
		enum Values
		{
			VALUE_LAST_BYTE = 255,
			VALUE_NEW = 256,
			VALUE_EOF = 257,
			VALUE_MAIN_NODE = 258
		};
	public:
		Table()
		{
			Reset();
		}
		void GetBits(int value, std::vector<Bits>& output)
		{
			auto& node = m_nodes[value];
			if (node.GetCount() < 0)
			{
				// issue new node
				RefreshBits(m_nodes[VALUE_NEW]);
				output.push_back(m_nodes[VALUE_NEW].GetBits());
				output.push_back(Bits(8, value));
				// add node with count 0
				Node& eof_node = m_nodes[VALUE_EOF];
				Node& new_node = m_nodes[m_used];
				Node& left_node = m_nodes[eof_node.GetLeft()];
				node.SetParent(m_used);
				node.SetLeft(VALUE_EOF);
				node.SetCount(0);
				new_node.SetValue(m_used);
				new_node.SetParent(eof_node.GetParent());
				new_node.SetLeft(left_node.GetValue());
				left_node.SetRight(m_used);
				new_node.SetRight(VALUE_EOF);
				new_node.SetCount(0);
				eof_node.SetParent(m_used);
				eof_node.SetLeft(m_used);
				eof_node.SetRight(value);
				eof_node.InvalidateBits(); // not needed?
				m_used++;
				// todo:
				//   after adding a node, check the height of the tree, it should be < 64 or a rebuild is needed.
			}
			else
			{
				// issue node
				RefreshBits(node);
				output.push_back(node.GetBits());
			}
			// increase node count
			IncreaseCount(node);
		}
	protected:
		void Reset()
		{
			for (int i = 0; i <= VALUE_LAST_BYTE; ++i)
			{
				m_nodes[i] = Node(-1, i, -1, -1, -1);
			}
			m_used = VALUE_MAIN_NODE;
			m_nodes[VALUE_NEW] = Node(1, VALUE_NEW, m_used, m_used, VALUE_EOF);
			m_nodes[VALUE_EOF] = Node(0, VALUE_EOF, m_used, VALUE_NEW, -1);
			m_nodes[m_used] = Node(1, m_used, -1, -1, VALUE_NEW);  // main node
			m_used++;
		}
		void IncreaseCount(Node& node)
		{
			auto parent = node.GetParent();
			if (parent >= 0)
			{
				IncreaseCount(m_nodes[parent]);
			}
			auto count = node.GetCount();
			node.SetCount(++count);
			for (;;)
			{
				auto left = node.GetLeft();
				if (left >= 0)
				{
					auto& leftNode = m_nodes[left];
					if (count > leftNode.GetCount())
					{
						SwapWithLeft(leftNode, node);
					}
					else
					{
						return;
					}
				}
				else
				{
					return;
				}
			}
		}
		void SwapWithLeft(Node& left, Node& right)
		{
			left.InvalidateBits();
			right.InvalidateBits();
			auto p = left.GetParent();
			left.SetParent(right.GetParent());
			right.SetParent(p);
			auto a = left.GetLeft();
			auto b = right.GetLeft();
			auto c = left.GetRight();
			auto d = right.GetRight();
			if (a >= 0)
			{
				m_nodes[a].SetRight(c);
			}
			right.SetLeft(a);
			right.SetRight(b);
			left.SetLeft(c);
			left.SetRight(d);
			if (d >= 0)
			{
				m_nodes[d].SetLeft(b);
			}
		}
		void RefreshBits(Node& node)
		{
			if (node.GetBits().GetLength() < 0)
			{
				if (node.GetParent() < 0)
				{
					node.SetBits(0, 0);
				}
				else
				{
					Node& parent = m_nodes[node.GetParent()];
					RefreshBits(parent);
					Bits bits = parent.GetBits();
					bits.AddBit(m_nodes[node.GetLeft()].GetParent() == node.GetParent());
					node.SetBits(bits);
				}
			}
		}
		void DumpNodesForDebug()
		{
			for (int i = 0; i < sizeof(m_nodes) / sizeof(*m_nodes); ++i)
			{
				Node& node = m_nodes[i];
				if (node.GetCount() >= 0)
				{
					cout << i
						<< " count=" << node.GetCount()
						<< " value=" << node.GetValue()
						<< " parent=" << node.GetParent()
						<< " left=" << node.GetLeft()
						<< " right=" << node.GetRight() << endl;
				}
			}
		}
	private:
		Node m_nodes[520];
		int m_used;
	};
public:
	class Compressor
	{
	public:
		void Work(void* data, int length)
		{
			m_output.reserve(length + m_output.size());
			for (int i = 0; i < length; ++i)
			{
				int value = static_cast<char*>(data)[i];
				m_table.GetBits(value, m_output);
			}
			m_table.GetBits(Table::VALUE_EOF, m_output);
		}
		size_t Flush(void *data,size_t /*bufferSize*/)
		{
			BitFiFo bb;
			for (const auto& bits : m_output)
			{
				bb.Push(bits.GetBits(), bits.GetLength());
			}
			std::vector<char> output;
			bb.Pop(output, true);
			memcpy(data, output.data(), output.size());
			return output.size();
		}
	private:
		Table m_table;
		std::vector<Bits> m_output;
	};
	class Decompressor
	{
	public:
		void Work(void* data, int length)
		{

		}
	private:
		Table m_table;
		std::vector<char> m_output;
	};
};

