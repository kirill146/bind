#include "gtest/gtest.h"
#include "bind.h"
#include <string>
#include <vector>
#include <algorithm>

struct my_struct {
	my_struct() {}
	my_struct(my_struct const&) {
		logger += "c";
	}
	my_struct(my_struct&&) {
		logger += "m";
	}
	static std::string logger;
};
std::string my_struct::logger;

void f(my_struct) {
	
}

void hw() {
	
}

TEST(testBind, helloWorld) {
	auto w = bind(hw);
	w();
}

TEST(testBind, moveFixedArgument) {
	my_struct::logger = "";
	auto w = bind(f, my_struct());
	EXPECT_EQ(my_struct::logger, "m");
	w();
	EXPECT_EQ(my_struct::logger, "mc");
}

TEST(testBind, copyFixedArgument) {
	my_struct::logger = "";
	my_struct x;
	auto w = bind(f, x);
	EXPECT_EQ(my_struct::logger, "c");
	w();
	EXPECT_EQ(my_struct::logger, "cc");
}

TEST(testBind, movePlaceholder) {
	my_struct::logger = "";
	auto w = bind(f, _1);
	EXPECT_EQ(my_struct::logger, "");
	w(my_struct());
	EXPECT_EQ(my_struct::logger, "m");
}

TEST(testBind, copyPlaceholder) {
	my_struct::logger = "";
	my_struct x;
	auto w = bind(f, _1);
	EXPECT_EQ(my_struct::logger, "");
	w(x);
	EXPECT_EQ(my_struct::logger, "c");
}

void g(my_struct, my_struct) {
	
}

TEST(testBind, copyUnuniquePlaceholder) {
	my_struct::logger = "";
	auto w = bind(g, _1, _1);
	EXPECT_EQ(my_struct::logger, "");
	w(my_struct());
	EXPECT_EQ(my_struct::logger, "cc");
}

void gg(my_struct, my_struct, my_struct) {
	
}

TEST(testBind, uniqueAndUnuniquePlaceholdersTogether) {
	my_struct::logger = "";
	auto w = bind(gg, _1, _2, _1);
	EXPECT_EQ(my_struct::logger, "");
	w(my_struct(), my_struct(), my_struct());
	EXPECT_TRUE(my_struct::logger == "mcc" || my_struct::logger == "cmc" || my_struct::logger == "ccm");
}

struct simpleFo {
	void operator()() {}
	simpleFo() = default;
	simpleFo(simpleFo const&) {
		logger += "c";
	}
	simpleFo(simpleFo&&) {
		logger += "m";
	}
	static std::string logger;
};
std::string simpleFo::logger;

TEST(testBind, moveSimpleFunctionalObject) {
	simpleFo::logger = "";
	auto w = bind(simpleFo());
	EXPECT_EQ(simpleFo::logger, "m");
	w();
	EXPECT_EQ(simpleFo::logger, "m");
}

TEST(testBind, copySimpleFunctionalObject) {
	simpleFo::logger = "";
	simpleFo x;
	auto w = bind(x);
	EXPECT_EQ(simpleFo::logger, "c");
	w();
	EXPECT_EQ(simpleFo::logger, "c");
}

bool comparator(int a, int b) {
	return bind([](int a, int b) -> bool {return a > b; }, _1, _2)(a, b);
}

TEST(testBind, correctness) {
	std::vector<int> v(1000);
	for (size_t i = 0; i < v.size(); i++) {
		v[i] = rand();
	}
	std::sort(v.begin(), v.end(), comparator);
	for (size_t i = 1; i < v.size(); i++) {
		EXPECT_TRUE(v[i - 1] >= v[i]);
	}
}

void bar(my_struct const) {

}

TEST(testBind, constFixedArgument) {
	my_struct::logger = "";
	auto w = bind(bar, my_struct());
	EXPECT_EQ(my_struct::logger, "m");
	w();
	EXPECT_EQ(my_struct::logger, "mc");
}

TEST(testBind, constPlaceholderArgument) {
	my_struct::logger = "";
	auto w = bind(bar, _1);
	EXPECT_EQ(my_struct::logger, "");
	w(my_struct());
	EXPECT_EQ(my_struct::logger, "m");
}

void foo(int a[]) {
	a[0] = 10;
	a[1] = 20;
}

int a[10];

TEST(testBind, arrayFixedArgument) {
	a[0] = a[1] = 0;
	auto w = bind(foo, a);
	w();
	EXPECT_EQ(a[0], 10);
	EXPECT_EQ(a[1], 20);
}

TEST(testBind, arrayPlaceholderArgument) {
	a[0] = a[1] = 0;
	auto w = bind(foo, _1);
	w(a);
	EXPECT_EQ(a[0], 10);
	EXPECT_EQ(a[1], 20);
}

my_struct& rec(my_struct& a, my_struct& b) {
	return a;
}

TEST(testBind, recBind) {
	my_struct::logger = "";
	my_struct x, y, z;
	auto w = bind(rec, bind(rec, x, y), z);
	EXPECT_EQ(my_struct::logger.length(), 5);
	my_struct::logger = "";
	w();
	EXPECT_EQ(my_struct::logger, "");
}

TEST(testBind, recBind2) {
	my_struct::logger = "";
	my_struct x, y;
	auto w = bind(rec, bind(rec, _1, _2), _1);
	EXPECT_EQ(my_struct::logger, "");
	w(x, y);
	EXPECT_EQ(my_struct::logger, "");
}

TEST(testCallOnceBind, moveFixedRvalueArgument) {
	my_struct::logger = "";
	auto w = call_once_bind(f, my_struct());
	EXPECT_EQ(my_struct::logger, "m");
	w();
	EXPECT_EQ(my_struct::logger, "mm");
}

TEST(testCallOnceBind, moveFixedLvalueArgument) {
	my_struct::logger = "";
	my_struct x;
	auto w = call_once_bind(f, x);
	EXPECT_EQ(my_struct::logger, "c");
	w();
	EXPECT_EQ(my_struct::logger, "cm");
}
