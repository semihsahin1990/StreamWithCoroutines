#include <streamc/experiment/ExpData.h>

#include <boost/filesystem.hpp>
#include <stdexcept>

using namespace std;
using namespace streamc::experiment;

ExpData::ExpData(string const & name, string const & dirName /*="."*/)
	: keepValues_(false),
	  currentFieldIndex_(0),
	  name_ (name),
	  dirName_ (dirName),
	  ext_(".dat"),
	  fileName_(name_+ext_)
{} 

void ExpData::setDescription(std::string const & desc)
{
	description_ = desc;
}

void ExpData::setKeepValues(bool keep) {
	keepValues_ = keep;
}

void ExpData::addFieldName(string const & field)
{
	fieldNameIndices_.insert(make_pair(field, fieldNames_.size()));
    fieldNames_.push_back(field);
}

void ExpData::open()
{
	using namespace boost::filesystem;
	path dir(dirName_);
	create_directory(dir);
	file_.open((dirName_+"/"+fileName_).c_str(), ios::out);
	if (!description_.empty())
		file_ << "# " << description_ << endl;
	file_ << "# ";
	for (size_t i=0, iu=fieldNames_.size(); i<iu; ++i)
		file_ << fieldNames_[i] << "\t";
	file_.flush();
}

void ExpData::addNewRecord()
{
	file_ << endl;
	if (keepValues_) {
		currentFieldIndex_ = 0;
		values_.push_back(vector<any_value>(fieldNames_.size(), any_value()));
	}
}

void ExpData::appendNewFieldValue(any_value const & value)
{
	addNewFieldValue(-1, value);
}

void ExpData::addNewFieldValue(std::string const & name, any_value const & value)
{
	if (fieldNameIndices_.count(name)==0) 
		throw runtime_error("field '"+name+"' not found!");
	size_t index = fieldNameIndices_[name];
	addNewFieldValue(index, value);
}

void ExpData::addNewFieldValue(int index, any_value const & value)
{
	file_ << value << "\t";
	file_.flush();
	if (keepValues_) {
		if (index < 0) 
			values_.back()[currentFieldIndex_++] = value;
		else
			values_.back()[static_cast<size_t>(index)] = value;
	}
}

void ExpData::close()
{
	file_.close();
}

vector<vector<ExpData::any_value> > const & ExpData::getValues() const
{
	return values_;
}


