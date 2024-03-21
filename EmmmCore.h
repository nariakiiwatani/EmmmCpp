#pragma once

#include <json.hpp>
#include <random>

namespace emmm {
using Json = nlohmann::json;

template<typename T>
class EmmmValue
{
public:
	EmmmValue(){}
	virtual T getValue() const = 0;
};

template<typename T>
static std::shared_ptr<EmmmValue<T>> create(const Json &json);

namespace utils {
template<typename T>
T get(const Json &json, std::string key, T default_value)
{
	if(json.contains(key)) {
		return json[key];
	}
	return default_value;
}
}

namespace detail {
template<typename T>
class EmmmInvalidType : public EmmmValue<T>
{
public:
	EmmmInvalidType(const Json &) {
		throw std::runtime_error("EmmmInvalidType instantiated with type: " + std::string(typeid(T).name()));
	}
	T getValue() const override {
		throw std::runtime_error("EmmmInvalidType::getValue called with type: " + std::string(typeid(T).name()));
	}
};
}

template<typename T>
class EmmmConstant : public EmmmValue<T>
{
public:
	EmmmConstant(const Json &json) {
		setValue(json.get<T>());
	}
	void setValue(const T &v) {
		value_ = v;
	}
	T getValue() const override { return value_; }
private:
	T value_;
};


namespace detail {
static std::mt19937& random_engine() {
	static std::random_device seed_gen;
	static std::mt19937 engine = std::mt19937(seed_gen());
	return engine;
}
template<typename T>
class RangeImpl : public EmmmValue<T>
{
public:
	RangeImpl(const Json &json) {
		setRange(json[0].get<T>(), json[1].get<T>());
	}
	RangeImpl(const T &a) {
		setRange(0, a);
	}
	RangeImpl(const T &a, const T &b) {
		setRange(a, b);
	}
	void setRange(const T &a, const T &b) {
		dist_ = distribution(a, b);
	}
	T getValue() const override { return dist_(random_engine()); }
private:
	using distribution = typename std::conditional<std::is_integral<T>::value, std::uniform_int_distribution<T>, std::uniform_real_distribution<T>>::type;
	mutable distribution dist_;
};
}

template<typename T>
using EmmmRange = typename std::conditional<std::is_arithmetic<T>::value, detail::RangeImpl<T>, detail::EmmmInvalidType<T>>::type;

template<typename T>
class EmmmChoice : public EmmmValue<T>
{
public:
	EmmmChoice(const Json &json) {
		if(!json.is_array()) {
			throw std::runtime_error("[EmmmChoice] invalid JSON format: must be an array");
		}
		float weight_sum = 0;
		for(int i = 0; i < json.size(); ++i) {
			auto &&data = json[i];
			float weight = utils::get<float>(data, "weight", 1);
			std::shared_ptr<EmmmValue<T>> value = create<T>(data);
			option_.push_back(Option{weight, value});
			weight_sum += weight;
		}
		selector_ = std::make_shared<EmmmRange<float>>(weight_sum);
	}
	T getValue() const override {
		if(option_.empty()) {
			throw std::runtime_error("[EmmmChoice] no option is provided");
		}
		float position = selector_->getValue();
		auto it = option_.cbegin();
		while(it != option_.cend() && position > it->weight) {
			position -= it->weight;
			++it;
		}
		if(it == option_.end()) {
			--it;
		}
		return it->value->getValue();
	}
private:
	struct Option {
		float weight;
		std::shared_ptr<EmmmValue<T>> value;
	};
	std::vector<Option> option_;
	std::shared_ptr<EmmmRange<float>> selector_;
};

template<typename T>
std::shared_ptr<EmmmValue<T>> create(const Json &json)
{
	std::shared_ptr<EmmmValue<T>> ret;
	if(json.is_object()) {
		if(!ret && json.contains("const")) ret = std::make_shared<EmmmConstant<T>>(json["const"]);
		if(!ret && json.contains("range")) ret = std::make_shared<EmmmRange<T>>(json["range"]);
		if(!ret && json.contains("choice")) ret = std::make_shared<EmmmChoice<T>>(json["choice"]);
		if(!ret && json.contains("value")) ret = create<T>(json["value"]);
		if(ret && utils::get<bool>(json, "initOnly", false)) {
			ret = std::make_shared<EmmmConstant<T>>(ret->getValue());
		}
	}
	else if(json.is_array()) {
		ret = std::make_shared<EmmmChoice<T>>(json);
	}
	else {
		ret = std::make_shared<EmmmConstant<T>>(json);
	}
	return ret;
}
}
