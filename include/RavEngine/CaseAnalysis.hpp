#pragma once

template<typename ... Ts>
struct CaseAnalysis : Ts ... {
	using Ts::operator() ...;
};
template<class... Ts> CaseAnalysis(Ts...) -> CaseAnalysis<Ts...>;

// use like this:
// std::visit(CaseAnalysis([](Type1 t1){ ... }, [](Type2 t2){ ... }), your_variant);