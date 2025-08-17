# 使用 popcount 使得两个集合完全相等的最小操作次数

```
v2
created, 2025-08-17T15:56+08:00
published, 2025-08-17T16:27+08:00
category: algorithm-problems
```

给定两个长度相同的数组 A 和 B，通过将元素转换为其二进制表示中 1 的个数（每次转换记为 1 次操作），
计算使两个数组中不匹配的元素变得匹配所需的最少操作次数。

不需要考虑元素匹配的顺序。

```bash
input:

5 3 7
3 3 5

output:
1
```

解释：直接将 A 中的 7 换成 3 即可，只需要 1 次变化，A 变换后为 `[5,3,3]`，B 是 `[3,3,5]`，题目说不需要考虑顺序，所以一次变换就够了。

这题是在美团一次机试中出的题目，有一种可以证明正确性的做法是，我们计算一个矩阵 M，`M[i][j]` 表示 `A[i]` 到 `B[j]` 所需变换次数。

然后这就是一个图的匹配问题了。

后来我想到一个贪心的算法：
因为两个数组中，最大且唯一的的那个元素，比如 sample 中的是 7，只在 A 中出现。
最大的元素一定要被 popcount 的，所以我们把 7 popcount 掉，然后塞回 A。

直到 A 和 B 相等。

唯一的问题是，这个策略是最优的吗？不妨假设 A 中最大为 x，B 中最大为 y，`x == y`，
假设最后 x 和 y 没有匹配上，最后的匹配结果是 x 经过 m 次匹配到了 B 中的 b, y 经过 n 次匹配到了 A 中的 a。

这样总共就是 n+m 次匹配。

如果直接让 x 和 y 配对，a 和 b 配对，最后 也只需要 `|n-m|` 次匹配。

所以

1. 如果数组 A 和 B 有两个相等的数，应该让他们直接配对，他俩可以直接从数组中移除掉
2. 对于 A U B 中最大的那个孤零零的数，一定要把它 popcount

[美团 0816 秋招笔试真题解析](https://mp.weixin.qq.com/s/3ypuE6SUSUydwQVoi0w72Q) 评论区有人说种做法可以 ak。

附代码，简直是短小精悍：

```cpp
#include <bit>
#include <cstdint>
#include <iostream>
#include <queue>
using namespace std;

int calCount(priority_queue<uint32_t> &a, priority_queue<uint32_t> &b) {
  int count = 0;
  while (!a.empty() && !b.empty()) {
    auto x = a.top();
    auto y = b.top();
    if (x == y) {
      a.pop();
      b.pop();
    } else if (x > y) {
      a.pop();
      a.push(std::popcount(x));
      count++;
    } else {
      b.pop();
      b.push(std::popcount(y));
      count++;
    }
  }
  return count;
}

int main() {
  int n;
  std::cin >> n;
  while (n-- > 0) {
    int len;
    cin >> len;
    priority_queue<uint32_t> a, b;
    for (int i = 0; i < len; i++) {
      uint32_t x;
      cin >> x;
      a.push(x);
    }
    for (int i = 0; i < len; i++) {
      uint32_t x;
      cin >> x;
      b.push(x);
    }
    cout << calCount(a, b) << endl;
  }
  return 0;
}
```
