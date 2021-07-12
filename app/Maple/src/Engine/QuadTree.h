//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine									//
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////


#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include <unordered_map>

namespace Maple 
{
	struct LeftOver
	{
		int16_t l; //left
		int16_t r; //right
		int16_t t; //top
		int16_t b; //bottom
		LeftOver(int16_t x, int16_t y, int16_t width, int16_t height)
		{
			l = x;
			r = x + width;
			t = y;
			b = y + height;
		}

		LeftOver()
		{
			l = 0;
			r = 0;
			t = 0;
			b = 0;
		}

		auto width() const
		{
			return r - l;
		}

		auto height() const
		{
			return b - t;
		}
	};


	template <typename K, typename V>
	class QuadTree
	{
	public:
		enum Direction
		{
			LEFT,
			RIGHT,
			UP,
			DOWN
		};

		QuadTree(std::function<Direction(const V&, const V&)> c)
		{
			root = 0;
			comparator = c;
		}

		QuadTree()
			: QuadTree(nullptr)
		{
		}

		auto clear()
		{
			nodes.clear();

			root = 0;
		}

		auto add(K key, V value)
		{
			K parent = 0;
			if (root)
			{
				K current = root;
				while (current)
				{
					parent = current;
					current = nodes[parent].addOrNext(key, value, comparator);
				}
			}
			else
			{
				root = key;
			}

			nodes.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(key),
				std::forward_as_tuple(value, parent, 0, 0, 0, 0)
			);
		}

		auto erase(K key)
		{
			if (!nodes.count(key))
				return;

			Node& toerase = nodes[key];

			std::vector<K> leaves;
			for (size_t i = LEFT; i <= DOWN; i++)
			{
				K leafkey = toerase[i];
				if (leafkey)
				{
					leaves.push_back(leafkey);
				}
			}

			K parent = toerase.parent;
			if (root == key)
			{
				root = 0;
			}
			else if (nodes.count(parent))
			{
				nodes[parent].erase(key);
			}

			nodes.erase(key);
			for (auto& leaf : leaves)
			{
				readd(parent, leaf);
			}
		}

		auto findNode(const V& value, std::function<bool(const V&, const V&)> predicate) -> K
		{
			if (root)
			{
				K key = findFrom(root, value, predicate);
				return predicate(value, nodes[key].value) ? key : 0;
			}
			return 0;
		}

		auto operator [](K key) -> V&

		{
			return nodes[key].value;
		}

		auto operator [](K key) const -> const V&

		{
			return nodes.at(key).value;
		}

	private:
		auto findFrom(K start, const V& value, std::function<bool(const V&, const V&)> predicate) -> K
		{
			if (!start)
				return 0;

			bool fulfilled = predicate(value, nodes[start].value);
			Direction dir = comparator(value, nodes[start].value);
			if (dir == RIGHT) //
			{
				K right = findFrom(nodes[start].right, value, predicate);
				if (right && predicate(value, nodes[right].value))
				{
					return right;
				}
				return start;
			}
			if (dir == DOWN)
			{
				K bottom = findFrom(nodes[start].bottom, value, predicate);
				if (bottom && predicate(value, nodes[bottom].value))
				{
					return bottom;
				}
				if (fulfilled)
				{
					return start;
				}
				K right = findFrom(nodes[start].right, value, predicate);
				if (right && predicate(value, nodes[right].value))
				{
					return right;
				}
				else
				{
					return start;
				}
			}
			if (dir == UP)
			{
				K top = findFrom(nodes[start].top, value, predicate);
				if (top && predicate(value, nodes[top].value))
				{
					return top;
				}
				else
				{
					if (fulfilled)
					{
						return start;
					}
					K right = findFrom(nodes[start].right, value, predicate);
					if (right && predicate(value, nodes[right].value))
					{
						return right;
					}
					return start;
				}
			}
			else
			{
				K left = findFrom(nodes[start].left, value, predicate);
				if (left && predicate(value, nodes[left].value))
				{
					return left;
				}
				if (fulfilled)
				{
					return start;
				}

				K bottom = findFrom(nodes[start].bottom, value, predicate);
				if (bottom && predicate(value, nodes[bottom].value))
				{
					return bottom;
				}
				else if (fulfilled)
				{
					return start;
				}

				K top = findFrom(nodes[start].top, value, predicate);
				if (top && predicate(value, nodes[top].value))
				{
					return top;
				}
				if (fulfilled)
				{
					return start;
				}

				K right = findFrom(nodes[start].right, value, predicate);
				if (right && predicate(value, nodes[right].value))
				{
					return right;
				}
				return start;
			}
		}

		auto readd(K start, K key) -> void
		{
			if (start)
			{
				K parent = 0;
				K current = start;
				while (current)
				{
					parent = current;
					current = nodes[parent].addOrNext(key, nodes[key].value, comparator);
				}

				nodes[key].parent = parent;
			}
			else if (start == root)
			{
				root = key;

				nodes[key].parent = 0;
			}
			else if (root)
			{
				readd(root, key);
			}
		}

		struct Node
		{
			V value;
			K parent;
			K left;
			K right;
			K top;
			K bottom;

			Node(const V& v, K p, K l, K r, K t, K b)
				: value(v), parent(p), left(l), right(r), top(t), bottom(b)
			{
			}

			Node()
				: Node(V(), 0, 0, 0, 0, 0)
			{
			}

			auto erase(K key)
			{
				if (left == key)
					left = 0;
				else if (right == key)
					right = 0;
				else if (top == key)
					top = 0;
				else if (bottom == key)
					bottom = 0;
			}

			auto addOrNext(K key, V val, std::function<Direction(const V&, const V&)> comparator) -> K
			{
				Direction dir = comparator(val, value); //���㵱ǰ�ڵ�Ӧ�÷��ڸ��ڵ��ʲôλ��;
				K dirkey = leaf(dir);
				if (!dirkey)
				{
					switch (dir)
					{
					case LEFT:
						left = key;
						break;
					case RIGHT:
						right = key;
						break;
					case UP:
						top = key;
						break;
					case DOWN:
						bottom = key;
						break;
					}
				}
				return dirkey;
			}

			auto leaf(Direction dir) -> K
			{
				switch (dir)
				{
				case LEFT:
					return left;
				case RIGHT:
					return right;
				case UP:
					return top;
				case DOWN:
					return bottom;
				default:
					return 0;
				}
			}

			auto operator [](size_t d) -> K
			{
				auto dir = static_cast<Direction>(d);
				return leaf(dir);
			}
		};

		std::function<Direction(const V&, const V&)> comparator;
		std::unordered_map<K, Node> nodes;
		K root;
	};

};
