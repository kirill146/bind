#pragma once
#include <tuple>
#include <type_traits>

template <int N>
struct placeholder {};

constexpr placeholder<1> _1;
constexpr placeholder<2> _2;
constexpr placeholder<3> _3;

template<typename F, bool call_once, typename ... As>
struct bind_t;

template <bool b, typename A>
struct type_ret {
	typedef A& type;
};

template <typename A>
struct type_ret<true, A> {
	typedef std::remove_reference_t<A>&& type;
};

template <typename T>
struct do_not_remove_reference_from_array {
	typedef std::remove_reference_t<T> type;
};

template <typename T, size_t N>
struct do_not_remove_reference_from_array<T(&)[N]> {
	typedef T(&type)[N];
};

template <typename T>
using do_not_remove_reference_from_array_t = typename do_not_remove_reference_from_array<T>::type;

template <bool call_once, typename A>
struct G {
	template <typename AA>
	G(AA&& a) : a(std::forward<AA>(a)) {}

	template <typename ... Bs>
	typename type_ret<call_once, A>::type operator()(Bs&&...) {
		return static_cast<typename type_ret<call_once, A>::type>(a);
	}

	do_not_remove_reference_from_array_t<A> a;
};

template <typename F, bool call_once, typename ... As>
struct G<call_once, bind_t<F, call_once, As...>> {
	template <typename AA>
	G(AA&& f) : f(std::forward<AA>(f)) {}

	template <typename ... Bs>
	decltype(auto) operator()(Bs&&... bs) {
		return f(std::forward<Bs>(bs)...);
	}

	bind_t<F, call_once, As...> f;
};

template <bool call_once>
struct G<call_once, const placeholder<1>&> {
	G(const placeholder<1>&) {}

	template <typename B1, typename ... Bs>
	decltype(auto) operator()(B1&& b1, Bs&&...) {
		return std::forward<B1>(b1);
	}
};

template <int N, bool call_once>
struct G<call_once, const placeholder<N>&> {
	G(const placeholder<N>&) {}

	template <typename B, typename ... Bs>
	decltype(auto) operator()(B&&, Bs&&... bs) {
		return G<call_once, const placeholder<N - 1>&>(placeholder<N - 1>())(std::forward<Bs>(bs)...);
	}
};

template <typename T, bool AsRvalue>
struct get_type {
	typedef T& type;
};

template <typename T>
struct get_type<T, true> {
	typedef T&& type;
};

template <typename T, bool AsRvalue>
using get_type_t = typename get_type<T, AsRvalue>::type;


template <int searching_num, typename ... As>
struct get_cnt {
	static constexpr int val = 0;
};

template <int searching_num, typename A0, typename ... As>
struct get_cnt<searching_num, A0, As...> {
	static constexpr int val = get_cnt<searching_num, As...>::val + std::is_same<const placeholder<searching_num>&, A0>::value;
};

template <int searching_num, typename F, bool call_once, typename ... AAs, typename ... As>
struct get_cnt<searching_num, bind_t<F, call_once, AAs...>, As...> {
	static constexpr int val = get_cnt<searching_num, As...>::val + get_cnt<searching_num, AAs...>::val;
};

template <typename F>
struct func_type {
	typedef std::remove_reference_t<F> type;
};

template <typename T, typename ... Args>
struct func_type<T(&)(Args...)> {
	typedef T(*type)(Args...);
};

template <typename F>
using func_type_t = typename func_type<F>::type;

template<typename F, bool call_once, typename ... As>
struct bind_t {
	template <typename FF, typename ... AAs>
	bind_t(FF&& f, AAs&&... as)
		: f(std::forward<FF>(f))
		, gs(std::forward<AAs>(as)...)
	{}

	template <typename ... Bs>
	decltype(auto) operator()(Bs&&... bs) {
		return call(std::make_integer_sequence<int, sizeof...(As)>(), std::make_integer_sequence<int, sizeof...(Bs)>(), std::forward<Bs>(bs)...);
	}

private:
	template <int... ks, int... ns, typename ... Bs>
	decltype(auto) call(std::integer_sequence<int, ks...>, std::integer_sequence<int, ns...>, Bs&&... bs) {
		return f(std::get<ks>(gs)(static_cast<get_type_t<Bs, get_cnt<ns + 1, As...>::val <= 1>>(bs)...)...);
	}

	func_type_t<F> f;
	std::tuple<G<call_once, As>...> gs;
};

template <typename F, typename ... As>
decltype(auto) bind(F&& f, As&&... as) {
	return bind_t<F, false, As...>(std::forward<F>(f), std::forward<As>(as)...);
}

template <typename F, typename ... As>
decltype(auto) call_once_bind(F&& f, As&&... as) {
	return bind_t<F, true, As...>(std::forward<F>(f), std::forward<As>(as)...);
}