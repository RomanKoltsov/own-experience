#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <deque>
#include <thread>

//typedef std::vector <std::int64_t> data_t;
typedef std::deque <std::int64_t> data_t;

struct data_compare {
	template <typename Data>
	static void execute (Data& lhs, const Data& rhs, std::ofstream& excluded, std::ofstream& added) {
		typename Data::const_iterator it = lhs.begin (), end = lhs.end ();
		typename Data::const_iterator _it = rhs.begin (), _end = rhs.end ();

		while (it != end && _it != _end) {
			if (*it == *_it) {
				++it;
				++_it;
			} else if (*it < *_it) {
				excluded << transform_to_string (*it) << std::endl;
				++it;
			} else {
				added << transform_to_string (*_it) << std::endl;
				++_it;
			}
		}

		for (; it != end; ++it) {
			excluded << transform_to_string(*it) << std::endl;
		}

		for (; _it != _end; ++_it) {
			added << transform_to_string(*_it) << std::endl;
		}
	}

	template <typename Data>
	static std::string transform_to_string (Data val) {
		std::string ret = std::to_string (val);
		if (ret.size () < 10) {
			std::string prefix (10 - ret.size (), '0');
			ret = prefix + ret;
		}
		ret.insert (4, ",");
		return ret;
	}
};

class Validator {
public:
	Validator (const std::string& file_name) {
		std::cout << "Validator: " << file_name << std::endl;
		m_ifs.open (file_name.c_str (), std::ios_base::in);
		m_ofs_fix.open (std::string ("fix_") + file_name, std::ios_base::trunc);
	}

	virtual ~Validator () {
		m_ifs.close ();
		m_ofs_fix.close ();
	}

public:
	void execute (data_t& out) {
		if (!m_ifs || !m_ofs_fix) {
			std::cout << "can't open file" << std::endl;
			throw std::exception();
		}

		const size_t rec_size = 11;

		size_t sss = 0;

		std::string str;

		while (!m_ifs.eof()) {
			std::getline(m_ifs, str);

			if (str.size() == rec_size) {
				size_t pos = str.find(',');

				if (pos != std::string::npos) {
					if (pos != 4 || str.size() - pos - 1 != 6) {
						// invalid
					} else {
						std::string original_str = str;
						str.erase(pos, 1);

						if (std::find_if(str.begin(), str.end(), [](const auto &x) {return (x < '0' || x > '9');}) == str.end()) {
							try {
								std::int64_t val = std::stoll(str);

								if (val > 0) {
									out.push_back(val);
									m_ofs_fix << original_str << std::endl;
								} else {
									// invalid
								}
							} catch (...) {
								std::cout << "unknown exception: data_size = " << out.size() << std::endl;
							}
						} else {
							// invalid
						}
					}
				}
			} else {
				// invalid
			}

			++sss;

			if (sss > 120000000) {
				break;
			}
		}

		std::sort (out.begin (), out.end ());
		out.erase (std::unique (out.begin (), out.end ()), out.end ());
	}

private:
	std::ifstream m_ifs;
	std::ofstream m_ofs_fix;

};

void validate_func (const std::string& file_name, data_t& data) {
	Validator obj (file_name);
	obj.execute (data);
}

int main (int argc, char* argv []) {
	data_t data1, data2;

	/**/
	{
		Validator obj ("list_of_expired_passports.csv");
		obj.execute (data1);
	}

	std::cout << "data_size = " << data1.size () << std::endl;

	{
		Validator obj ("list_of_expired_passports (1).csv");
		obj.execute (data2);
	}

	std::cout << "data_size = " << data2.size () << std::endl;
	/**/

	/**
	std::thread thr1 (validate_func, "list_of_expired_passports.csv", data1);
	thr1.join();

	std::thread thr2 (validate_func, "list_of_expired_passports (1).csv", data2);
	thr2.join();

	std::cout << "data_size = " << data1.size () << std::endl;
	std::cout << "data_size = " << data2.size () << std::endl;
	/**/

	std::cout << "compare.." << std::endl;

	std::ofstream excluded ("excluded", std::ios_base::trunc);
	std::ofstream added ("added", std::ios_base::trunc);

	try {
		if (excluded) {
			if (added) {
				data_compare::execute(data1, data2, excluded, added);
			} else {
				std::cout << "can't open 'added' file" << std::endl;
			}
		} else {
			std::cout << "can't open 'excluded' file" << std::endl;
		}
	} catch (...) {
		std::cout << "unknown exception\n";
	}

	excluded.close ();
	added.close ();

	std::cout << "finish\n";

	return 0;
}
