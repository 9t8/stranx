#include "datum.h"

#include <sstream>

p_datum procedure::call(const p_datum &args, environment &env) const {
	environment new_env(create_new_env(args, env));

	const pair *exprs(dynamic_cast<const pair *>(body.get()));
	p_datum result(next(exprs)->eval(new_env));
	while (exprs) {
		result = next(exprs)->eval(new_env);
	}
	return result;
}

environment procedure::create_new_env(const p_datum &args, environment &env) const {
	environment new_env(env);

	const pair *curr_arg(dynamic_cast<const pair *>(args.get()));

	if (formals.empty()) {
		assert(!curr_arg && "too many arguments");

		return new_env;
	}

	for (size_t i(0); i < formals.size() - 1; ++i) {
		new_env[formals[i]] = next(curr_arg)->eval(env);
	}

	if (!variadic) {
		new_env[formals.back()] = next(curr_arg)->eval(env);
		assert(!curr_arg && "too many arguments");

		return new_env;
	}

	if (!curr_arg) {
		new_env[formals.back()] = std::make_shared<empty_list>();
		return new_env;
	}

	std::shared_ptr<pair> tail(std::make_shared<pair>(next(curr_arg)->eval(env)));
	new_env[formals.back()] = tail;
	while (curr_arg) {
		std::shared_ptr<pair> new_tail(std::make_shared<pair>(next(curr_arg)->eval(env)));
		tail->cdr = new_tail;
		tail = new_tail;
	}
	return new_env;
}

bool pair::stringify_into_lists(true);

pair::operator std::string() const {
	if (!stringify_into_lists) {
		return "(" + static_cast<std::string>(*car) + " . " + static_cast<std::string>(*cdr)
			   + ")";
	}

	std::ostringstream oss;
	p_datum p_last(cdr);
	const pair *curr_pair(this);

	oss << "(" << *next(curr_pair);
	while (curr_pair != nullptr) {
		p_last = curr_pair->cdr;
		oss << " " << *next(curr_pair);
	}
	const datum &last(*p_last);
	if (typeid(last) != typeid(empty_list)) {
		oss << " . " << last;
	}
	oss << ")";

	return oss.str();
}
