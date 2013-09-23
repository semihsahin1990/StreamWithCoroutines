#pragma once

#include<string>
#include<iostream>
#include<inttypes.h>
#include<unordered_map>
#include<vector>

class Tuple{

private:
	class Value{
		enum Type {Integer, Double, String, IntList, DoubleList, StringList};
	public:
		Type type;

		Value(int64_t const &value){
			type = Integer;
			pointer.intPointer = new int64_t(value);
		}
		
		Value(double const &value){
			type = Double;
			pointer.doublePointer = new double(value);
		}

		Value(std::string const &value){
			type = String;
			pointer.stringPointer = new std::string(value);
		}

		Value(std::vector<int64_t> const &list){
			type = IntList;
			pointer.intListPointer = new std::vector<int64_t>(list);
		}

		Value(std::vector<double> const &list){
			type = DoubleList;
			pointer.doubleListPointer = new std::vector<double>(list);
		}

		Value(std::vector<std::string> const &list){
			type = StringList;
			pointer.stringListPointer = new std::vector<std::string>(list);
		}

		~Value(){
			switch(type){
				case Integer:
					delete pointer.intPointer;
				break;

				case Double:
					delete pointer.doublePointer;
				break;

				case String:
					delete pointer.stringPointer;
				break;

			}
		}

		int64_t &getIntValue(){
			return *pointer.intPointer;
		}

		double &getDoubleValue(){
			return *pointer.doublePointer;
		}

		std::string &getStringValue(){
			return *pointer.stringPointer;
		}
		
		std::vector<int64_t> &getIntList(){
			return *pointer.intListPointer;
		}

		std::vector<double> &getDoubleList(){
			return *pointer.doubleListPointer;
		}

		std::vector<std::string> &getStringList(){
			return *pointer.stringListPointer;
		}

	private:
		union Ptr{
			int64_t *intPointer;
			std::string *stringPointer;
			double *doublePointer;
			std::vector<int64_t> *intListPointer;
			std::vector<double> *doubleListPointer;
			std::vector<std::string> *stringListPointer;
		} pointer;
	};

	std::unordered_map<std::string, Value *> values;

public:
	~Tuple(){
		//delete all
	};

	void addAttribute(std::string const &name, int64_t const &value){
		Value *val = new Value(value);
		values[name] = val;
	}

	void addAttribute(std::string const &name, double const &value){
		Value *val = new Value(value);
		values[name] = val;
	}

	void addAttribute(std::string const &name, std::string const &value){
		Value *val = new Value(value);
		values[name] = val;
	}

	void addAttribute(std::string const &name, std::vector<int64_t> const &list){
		Value *val = new Value(list);
		values[name] = val;
	}

	void addAttribute(std::string const &name, std::vector<double> const &list){
		Value *val = new Value(list);
		values[name] = val;
	}

	void addATtribute(std::string const &name, std::vector<std::string> const &list){
		Value *val = new Value(list);
		values[name] = val;
	}

	int64_t &getIntAttribute(std::string const &name){
		return values[name]->getIntValue();
	}

	double &getDoubleAttribute(std::string const &name){
		return values[name]->getDoubleValue();
	}

	std::string &getStringAttribute(std::string const &name){
		return values[name]->getStringValue();
	}

	std::vector<int64_t> &getIntListAttribute(std::string const &name){
		return values[name]->getIntList();
	}

	std::vector<double> &getDoubleListAttribute(std::string const &name){
		return values[name]->getDoubleList();
	}

	std::vector<std::string> &getStringListAttribute(std::string const &name){
		return values[name]->getStringList();
	}

};
