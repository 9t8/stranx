#include "func.h"
#include "lexer.h"

#include <iostream>

using namespace stranx;

static sp<datum> parse_datum(const tok_list &toks, size_t &idx) {
	const auto peek_next_type([&]() -> const std::type_info & {
		assert(idx < toks.size() &&
		"expected a token but none found (too many opening parens?)");

		tok &next_tok(*toks[idx]);
		return typeid(next_tok);
	});

	if (peek_next_type() == typeid(beginl)) {
		++idx;
		if (peek_next_type() == typeid(endl)) {
			++idx;
			return std::make_shared<emptyl>();
		}

		sp<pair> p(std::make_shared<pair>(parse_datum(toks, idx)));
		sp<datum> start(p);
		while (peek_next_type() != typeid(endl)) {
			if (peek_next_type() == typeid(dot)) {
				p->cdr = parse_datum(toks, ++idx);
				assert(peek_next_type() == typeid(endl) && "misplaced dot token");
				break;
			}
			sp<pair> p_new(std::make_shared<pair>(parse_datum(toks, idx)));
			p->cdr = p_new;
			p = p_new;
		}
		++idx;

		return start;
	}
	if (peek_next_type() == typeid(quote_tok)) {
		return std::make_shared<quote>(parse_datum(toks, ++idx));
	}

	sp<datum> p(std::dynamic_pointer_cast<datum>(toks.at(idx++)));
	assert(p && "tried to parse an unparsable token");

	return p;
}

static sp<datum> lambda(const sp<datum> &args, const sp<env> &curr_env) {
	const datum &temp(*args);
	assert(typeid(temp) == typeid(pair) && "not enough arguments");
	const pair &args_list(dynamic_cast<const pair &>(temp));

	std::vector<std::string> formals;

	sp<iden> variadic_iden(std::dynamic_pointer_cast<iden>(args_list.car));
	sp<pair> curr_formal(std::dynamic_pointer_cast<pair>(args_list.car));
	while (curr_formal) {
		variadic_iden = std::dynamic_pointer_cast<iden>(curr_formal->cdr);

		const datum &formal_iden(*next(curr_formal));
		assert(typeid(formal_iden) == typeid(iden) &&
			   "all formals must be identifiers (variadics are not supported)");

		formals.push_back(dynamic_cast<const iden &>(formal_iden).name);
	}
	if (variadic_iden) {
		formals.push_back(variadic_iden->name);
	}

	const sp<pair> body(std::dynamic_pointer_cast<pair>(args_list.cdr));
	assert(body && "invalid procedure body");

	return std::make_shared<closure>(formals, variadic_iden.get(), body, curr_env);
}

static sp<datum> define(const sp<datum> &args, const sp<env> &curr_env) {
	const datum &temp(*args);
	assert(typeid(temp) == typeid(pair) && "not enough arguments");
	const pair &args_list(dynamic_cast<const pair &>(temp));

	const datum &temp1(*args_list.cdr);
	assert(typeid(temp1) == typeid(pair) && "not enough arguments");
	const pair &cdr(dynamic_cast<const pair &>(temp1));

	const datum &car(*args_list.car);

	if (typeid(car) == typeid(iden)) {
		const datum &cddr(*cdr.cdr);
		assert(typeid(cddr) == typeid(emptyl) && "too many arguments");

		curr_env->define(dynamic_cast<const iden &>(*args_list.car).name,
						 cdr.car->eval(curr_env));
	} else {
		assert(typeid(car) == typeid(pair) && "first argument must be identifier or list");

		const pair &formals(dynamic_cast<const pair &>(*args_list.car));

		const datum &caar(*formals.car);
		assert(typeid(caar) == typeid(iden) && "procedure name must be an identifier");

		const sp<datum> lambda(curr_env->find("lambda"));
		assert(dynamic_cast<const func *>(lambda.get()) &&
			   "lambda was redefined into an uncallable object");

		curr_env->define(dynamic_cast<const iden &>(*formals.car).name,
						 dynamic_cast<const func &>(*lambda).call(std::make_shared<pair>(
									 formals.cdr, args_list.cdr), curr_env));
	}

	return std::make_shared<emptyl>();
}

static sp<datum> quote_func(const sp<datum> &args, const sp<env> &) {
	const datum &temp(*args);
	assert(typeid(temp) == typeid(pair) && "not enough arguments");
	const pair &args_list(dynamic_cast<const pair &>(temp));

	const datum &cdr(*args_list.cdr);
	assert(typeid(cdr) == typeid(emptyl) && "too many arguments");

	return args_list.car;
}

int main(int, const char *[]) {
	tok_list toks(lexer(std::cin).lex());

	std::cout << "===-- toks --===\n";
	for (size_t i(0); i < toks.size(); ++i) {
		std::cout << " " << *toks[i];
	}
	std::cout << "\n";

	std::vector<sp<datum>> tree;
	for (size_t i(0); i < toks.size();) {
		tree.push_back(parse_datum(toks, i));
	}

	std::cout << "\n===-- tree --===\n";
	for (size_t i(0); i < tree.size(); ++i) {
		std::cout << *tree[i] << "\n";
	}
	std::cout << "\n===-- output --===" << std::endl;

	const sp<env> top_level(std::make_shared<env>(nullptr));
	top_level->define("lambda", std::make_shared<native_func>(lambda));
	top_level->define("define", std::make_shared<native_func>(define));
	top_level->define("quote", std::make_shared<native_func>(quote_func));

	for (size_t i(0); i < tree.size(); ++i) {
		std::cout << *tree[i]->eval(top_level) << std::endl;
	}
}
